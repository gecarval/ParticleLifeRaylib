#include "GravityServer.hpp"
#include "../node/canvas_item/node2d/collision_object2d/physics_body2d/PhysicsBody2D.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <vector>

// ===========================================================================
// Implementation notes
// ---------------------------------------------------------------------------
// This is a classic 2D Fast Multipole Method built on complex-plane potential
// theory. A point mass m at z_i contributes m*log(z - z_i) to the (complex)
// gravitational potential; summing over all sources and differentiating
// gives the field used to accelerate every other body. That kernel is
// exactly what a 2D multipole expansion is built from, which is why complex
// arithmetic is used throughout instead of raw (x, y) pairs.
//
// Pipeline per call to applyGravity():
//   P2M  - each quadtree leaf builds a multipole expansion from its bodies.
//   M2M  - child expansions are translated and summed into their parent.
//   M2L  - during a dual-tree traversal, well separated cell pairs convert
//          each other's multipole expansion into a local expansion.
//   L2L  - local expansions are pushed down from parent to child.
//   P2P  - bodies in nearby (non well separated) leaves are summed directly.
//   L2P  - each leaf evaluates its final local expansion at its bodies.
//
// Everything below the public GravityServer methods is an implementation
// detail confined to this translation unit (anonymous namespace + a nested
// QuadNode type that is only forward-declared in the header), the same way
// PhysicsServer hides its hash grid behind a small public API.
// ===========================================================================

namespace {

constexpr int	kOrder = 6; // terms kept beyond the monopole (a_0)
constexpr int	kMaxPerLeaf = 8;
constexpr int	kMaxDepth = 32;
constexpr float kTheta = 0.6f; // smaller => more accurate, more direct work

using Cplx = std::complex<float>;

inline Cplx toComplex(const raylib::Vector2 &v) noexcept {
	return Cplx(v.x, v.y);
}

inline raylib::Vector2 fieldToAccel(const Cplx &omegaPrime,
									const float strength) noexcept {
	// phi = Re(sum m_i log(z - z_i)) is the (attractive) potential; the
	// physical gradient of an analytic function w = phi + i*psi is
	// (Re(w'), -Im(w')), and acceleration is -grad(phi).
	return raylib::Vector2(-strength * omegaPrime.real(),
						   strength * omegaPrime.imag());
}

double binomial(const int n, const int k) noexcept {
	if (k < 0 || k > n) {
		return 0.0;
	}
	double result = 1.0;
	for (int i = 0; i < k; ++i) {
		result =
			result * static_cast<double>(n - i) / static_cast<double>(i + 1);
	}
	return result;
}

std::array<Cplx, kOrder + 1> powersOf(const Cplx &z) noexcept {
	std::array<Cplx, kOrder + 1> result{};
	result[0] = Cplx(1.0f, 0.0f);
	for (int i = 1; i <= kOrder; ++i) {
		result[static_cast<std::size_t>(i)] =
			result[static_cast<std::size_t>(i - 1)] * z;
	}
	return result;
}

} // namespace

// ---------------------------------------------------------------------------
// QuadNode: adaptive quadtree cell holding either bodies (leaf) or four
// children, plus its multipole (source) and local (field) expansions.
// ---------------------------------------------------------------------------
struct GravityServer::QuadNode {
	raylib::Vector2							 center;
	float									 halfSize;
	bool									 isLeaf;
	std::vector<PhysicsBody2D *>			 bodies;
	std::array<std::unique_ptr<QuadNode>, 4> children;
	std::array<Cplx, kOrder + 1>			 multipole;
	std::array<Cplx, kOrder + 1>			 local;

	QuadNode(const raylib::Vector2 &c, const float hs)
		: center(c), halfSize(hs), isLeaf(true) {
		multipole.fill(Cplx(0.0f, 0.0f));
		local.fill(Cplx(0.0f, 0.0f));
	}
};

namespace {

using QuadNode = GravityServer::QuadNode;

// ---- tree construction ----------------------------------------------------
void splitNode(QuadNode &node, std::vector<PhysicsBody2D *> group,
			   const int depth) {
	if (static_cast<int>(group.size()) <= kMaxPerLeaf || depth >= kMaxDepth) {
		node.bodies = std::move(group);
		node.isLeaf = true;
		return;
	}
	node.isLeaf = false;

	std::array<std::vector<PhysicsBody2D *>, 4> buckets;
	for (PhysicsBody2D *body : group) {
		const raylib::Vector2 &p = body->getPos();
		const std::size_t	   idx = static_cast<std::size_t>(
			 (p.x >= node.center.x ? 1 : 0) | (p.y >= node.center.y ? 2 : 0));
		buckets[idx].push_back(body);
	}

	const float childHalf = node.halfSize * 0.5f;
	for (int i = 0; i < 4; ++i) {
		const std::size_t idx = static_cast<std::size_t>(i);
		if (buckets[idx].empty()) {
			continue;
		}
		const float cx = node.center.x + ((i & 1) ? childHalf : -childHalf);
		const float cy = node.center.y + ((i & 2) ? childHalf : -childHalf);
		node.children[idx] =
			std::make_unique<QuadNode>(raylib::Vector2(cx, cy), childHalf);
		splitNode(*node.children[idx], std::move(buckets[idx]), depth + 1);
	}
}

std::unique_ptr<QuadNode>
buildQuadTree(const std::vector<PhysicsBody2D *> &bodies) {
	if (bodies.empty()) {
		return nullptr;
	}
	float minX = bodies.front()->getPos().x;
	float maxX = minX;
	float minY = bodies.front()->getPos().y;
	float maxY = minY;
	for (PhysicsBody2D *body : bodies) {
		const raylib::Vector2 &p = body->getPos();
		minX = std::min(minX, p.x);
		maxX = std::max(maxX, p.x);
		minY = std::min(minY, p.y);
		maxY = std::max(maxY, p.y);
	}
	const raylib::Vector2 center((minX + maxX) * 0.5f, (minY + maxY) * 0.5f);
	const float			  halfSize =
		std::max({(maxX - minX) * 0.5f, (maxY - minY) * 0.5f, 1.0f}) + 1.0f;

	auto root = std::make_unique<QuadNode>(center, halfSize);
	splitNode(*root, bodies, 0);
	return root;
}

// ---- P2M / M2M (upward pass) ----------------------------------------------
void multipoleFromBodies(QuadNode &node) {
	const Cplx z0 = toComplex(node.center);

	Cplx total(0.0f, 0.0f);
	for (PhysicsBody2D *body : node.bodies) {
		total += Cplx(body->getMass(), 0.0f);
	}
	node.multipole[0] = total;

	std::array<Cplx, kOrder + 1> sums{};
	sums.fill(Cplx(0.0f, 0.0f));
	for (PhysicsBody2D *body : node.bodies) {
		const Cplx dz = toComplex(body->getPos()) - z0;
		const Cplx mass = Cplx(body->getMass(), 0.0f);
		Cplx	   power = dz;
		for (int k = 1; k <= kOrder; ++k) {
			sums[static_cast<std::size_t>(k)] += mass * power;
			power *= dz;
		}
	}
	for (int k = 1; k <= kOrder; ++k) {
		node.multipole[static_cast<std::size_t>(k)] =
			-sums[static_cast<std::size_t>(k)] / static_cast<float>(k);
	}
}

// Translate child's multipole expansion (about child.center) into parent's
// expansion (about parent.center), accumulating into parent.multipole.
void multipoleToMultipole(const QuadNode &child, QuadNode &parent) {
	const Cplx c = toComplex(child.center) - toComplex(parent.center);
	const auto cPow = powersOf(c);

	parent.multipole[0] += child.multipole[0];
	for (int l = 1; l <= kOrder; ++l) {
		Cplx term = -child.multipole[0] * cPow[static_cast<std::size_t>(l)] /
					static_cast<float>(l);
		for (int k = 1; k <= l; ++k) {
			term += child.multipole[static_cast<std::size_t>(k)] *
					cPow[static_cast<std::size_t>(l - k)] *
					static_cast<float>(binomial(l - 1, k - 1));
		}
		parent.multipole[static_cast<std::size_t>(l)] += term;
	}
}

void computeMultipoles(QuadNode &node) {
	if (node.isLeaf) {
		multipoleFromBodies(node);
		return;
	}
	for (auto &child : node.children) {
		if (child) {
			computeMultipoles(*child);
			multipoleToMultipole(*child, node);
		}
	}
}

// ---- M2L / L2L (downward pass) ---------------------------------------------
bool wellSeparated(const QuadNode &a, const QuadNode &b) noexcept {
	const float dx = b.center.x - a.center.x;
	const float dy = b.center.y - a.center.y;
	const float dist = std::sqrt(dx * dx + dy * dy);
	if (dist < 1e-6f) {
		return false;
	}
	const float span = (a.halfSize + b.halfSize) * 1.41421356f;
	return (span / dist) < kTheta;
}

// Convert source's multipole expansion into a local expansion about
// target.center, accumulating into target.local.
void multipoleToLocal(const QuadNode &source, QuadNode &target) {
	const Cplx d = toComplex(target.center) - toComplex(source.center);
	if (std::abs(d) < 1e-6f) {
		return;
	}
	const Cplx invD = Cplx(1.0f, 0.0f) / d;
	const auto invDPow = powersOf(invD);

	Cplx b0 = source.multipole[0] * std::log(d);
	for (int k = 1; k <= kOrder; ++k) {
		b0 += source.multipole[static_cast<std::size_t>(k)] *
			  invDPow[static_cast<std::size_t>(k)];
	}
	target.local[0] += b0;

	for (int l = 1; l <= kOrder; ++l) {
		const float sign = (l % 2 == 0) ? 1.0f : -1.0f; // (-1)^l
		Cplx		bl = -sign * source.multipole[0] *
				  invDPow[static_cast<std::size_t>(l)] / static_cast<float>(l);
		Cplx sum(0.0f, 0.0f);
		for (int k = 1; k <= kOrder; ++k) {
			sum += source.multipole[static_cast<std::size_t>(k)] *
				   static_cast<float>(binomial(l + k - 1, l)) *
				   invDPow[static_cast<std::size_t>(l)] *
				   invDPow[static_cast<std::size_t>(k)];
		}
		bl += sign * sum;
		target.local[static_cast<std::size_t>(l)] += bl;
	}
}

// Shift parent's local expansion (about parent.center) down into child
// (about child.center), accumulating into child.local.
void localToLocal(const QuadNode &parent, QuadNode &child) {
	const Cplx e = toComplex(child.center) - toComplex(parent.center);
	const auto ePow = powersOf(e);
	for (int m = 0; m <= kOrder; ++m) {
		Cplx cm(0.0f, 0.0f);
		for (int l = m; l <= kOrder; ++l) {
			cm += parent.local[static_cast<std::size_t>(l)] *
				  static_cast<float>(binomial(l, m)) *
				  ePow[static_cast<std::size_t>(l - m)];
		}
		child.local[static_cast<std::size_t>(m)] += cm;
	}
}

// ---- P2P direct summation --------------------------------------------------
void directPair(PhysicsBody2D *bi, PhysicsBody2D *bj, const float strength,
				std::unordered_map<unsigned long, raylib::Vector2> &accel) {
	const Cplx	dz = toComplex(bi->getPos()) - toComplex(bj->getPos());
	const float distSq = dz.real() * dz.real() + dz.imag() * dz.imag();
	if (distSq < 1e-6f) {
		return;
	}
	accel[bi->getInstanceID()] +=
		fieldToAccel(Cplx(bj->getMass(), 0.0f) / dz, strength);
	accel[bj->getInstanceID()] +=
		fieldToAccel(Cplx(bi->getMass(), 0.0f) / (-dz), strength);
}

// ---- dual-tree traversal: builds the M2L interaction lists and handles
// the near field (P2P) for every admissible / inadmissible cell pair -------
void interact(QuadNode &a, QuadNode &b, const float strength,
			  std::unordered_map<unsigned long, raylib::Vector2> &accel) {
	if (&a == &b) {
		if (a.isLeaf) {
			for (std::size_t i = 0; i < a.bodies.size(); ++i) {
				for (std::size_t j = i + 1; j < a.bodies.size(); ++j) {
					directPair(a.bodies[i], a.bodies[j], strength, accel);
				}
			}
			return;
		}
		for (auto &child : a.children) {
			if (child) {
				interact(*child, *child, strength, accel);
			}
		}
		for (int i = 0; i < 4; ++i) {
			if (!a.children[static_cast<std::size_t>(i)]) {
				continue;
			}
			for (int j = i + 1; j < 4; ++j) {
				if (!a.children[static_cast<std::size_t>(j)]) {
					continue;
				}
				interact(*a.children[static_cast<std::size_t>(i)],
						 *a.children[static_cast<std::size_t>(j)], strength,
						 accel);
			}
		}
		return;
	}

	if (wellSeparated(a, b)) {
		multipoleToLocal(b, a);
		multipoleToLocal(a, b);
		return;
	}

	if (a.isLeaf && b.isLeaf) {
		for (PhysicsBody2D *bi : a.bodies) {
			for (PhysicsBody2D *bj : b.bodies) {
				directPair(bi, bj, strength, accel);
			}
		}
		return;
	}

	// Subdivide the larger (or the non-leaf) side and recurse.
	if (b.isLeaf || (!a.isLeaf && a.halfSize >= b.halfSize)) {
		for (auto &child : a.children) {
			if (child) {
				interact(*child, b, strength, accel);
			}
		}
	} else {
		for (auto &child : b.children) {
			if (child) {
				interact(a, *child, strength, accel);
			}
		}
	}
}

// ---- L2L propagation + L2P evaluation --------------------------------------
void evaluateLocalAtLeaf(
	QuadNode &leaf, const float strength,
	std::unordered_map<unsigned long, raylib::Vector2> &accel) {
	const Cplx z0 = toComplex(leaf.center);
	for (PhysicsBody2D *body : leaf.bodies) {
		const Cplx dz = toComplex(body->getPos()) - z0;
		Cplx	   omegaPrime(0.0f, 0.0f);
		Cplx	   power(1.0f, 0.0f); // dz^(l-1)
		for (int l = 1; l <= kOrder; ++l) {
			omegaPrime += static_cast<float>(l) *
						  leaf.local[static_cast<std::size_t>(l)] * power;
			power *= dz;
		}
		accel[body->getInstanceID()] += fieldToAccel(omegaPrime, strength);
	}
}

void propagateAndEvaluate(
	QuadNode &node, const float strength,
	std::unordered_map<unsigned long, raylib::Vector2> &accel) {
	if (node.isLeaf) {
		evaluateLocalAtLeaf(node, strength, accel);
		return;
	}
	for (auto &child : node.children) {
		if (child) {
			localToLocal(node, *child);
			propagateAndEvaluate(*child, strength, accel);
		}
	}
}

// ---- treecode-style single point query (M2P) for single-body queries ------
Cplx fieldAt(const QuadNode &node, const raylib::Vector2 &pos,
			 const unsigned long excludeID) {
	if (node.isLeaf) {
		Cplx result(0.0f, 0.0f);
		for (PhysicsBody2D *body : node.bodies) {
			if (body->getInstanceID() == excludeID) {
				continue;
			}
			const Cplx	dz = toComplex(pos) - toComplex(body->getPos());
			const float distSq = dz.real() * dz.real() + dz.imag() * dz.imag();
			if (distSq < 1e-6f) {
				continue;
			}
			result += Cplx(body->getMass(), 0.0f) / dz;
		}
		return result;
	}

	const float dx = pos.x - node.center.x;
	const float dy = pos.y - node.center.y;
	const float dist = std::sqrt(dx * dx + dy * dy);
	if (dist > 1e-6f && (node.halfSize * 1.41421356f / dist) < kTheta) {
		const Cplx dz = toComplex(pos) - toComplex(node.center);
		const Cplx invDz = Cplx(1.0f, 0.0f) / dz;
		Cplx	   term = node.multipole[0] * invDz;
		Cplx	   pw = invDz;
		for (int k = 1; k <= kOrder; ++k) {
			pw *= invDz; // invDz^(k+1)
			term += -static_cast<float>(k) *
					node.multipole[static_cast<std::size_t>(k)] * pw;
		}
		return term;
	}

	Cplx result(0.0f, 0.0f);
	for (const auto &child : node.children) {
		if (child) {
			result += fieldAt(*child, pos, excludeID);
		}
	}
	return result;
}

} // namespace

// ===========================================================================
// GravityServer
// ===========================================================================

const int	GravityServer::EXPANSION_ORDER = kOrder;
const int	GravityServer::MAX_BODIES_PER_LEAF = kMaxPerLeaf;
const int	GravityServer::MAX_DEPTH = kMaxDepth;
const float GravityServer::THETA = kTheta;

GravityServer *GravityServer::_instance = nullptr;
std::unordered_map<unsigned long, PhysicsBody2D *> GravityServer::_bodies;

GravityServer::GravityServer(const std::string &instanceName)
	: Object(instanceName), _root(nullptr) {
}

GravityServer::~GravityServer() {
}

void GravityServer::addBody(PhysicsBody2D *body) noexcept {
	_bodies[body->getInstanceID()] = body;
}

void GravityServer::removeBody(PhysicsBody2D *body) noexcept {
	_bodies.erase(body->getInstanceID());
}

GravityServer &GravityServer::getInstance() noexcept {
	if (_instance == nullptr) {
		_instance = new GravityServer("GravityServer");
	}
	return *_instance;
}

void GravityServer::deleteInstance() noexcept {
	if (_instance != nullptr) {
		delete _instance;
		_instance = nullptr;
	}
}

// Core API
void GravityServer::rebuild() {
	std::vector<PhysicsBody2D *> bodies;
	bodies.reserve(_bodies.size());
	for (const auto &pair : _bodies) {
		bodies.push_back(pair.second);
	}
	_root = buildQuadTree(bodies);
	if (_root) {
		computeMultipoles(*_root);
	}
}

void GravityServer::applyGravity(const float strength) {
	if (!_root) {
		return;
	}
	std::unordered_map<unsigned long, raylib::Vector2> accel;
	accel.reserve(_bodies.size());

	interact(*_root, *_root, strength, accel);
	propagateAndEvaluate(*_root, strength, accel);

	for (const auto &pair : _bodies) {
		const auto it = accel.find(pair.first);
		if (it != accel.end()) {
			pair.second->applyForce(it->second * pair.second->getMass());
		}
	}
}

// Legacy
void GravityServer::applyGravity(PhysicsBody2D &body,
								 const float	strength) const {
	body.applyForce(getAcceleration(body, strength) * body.getMass());
}

raylib::Vector2 GravityServer::getAcceleration(PhysicsBody2D &body,
											   const float	  strength) const {
	if (!_root) {
		return raylib::Vector2(0.0f, 0.0f);
	}
	const Cplx omegaPrime =
		fieldAt(*_root, body.getPos(), body.getInstanceID());
	return fieldToAccel(omegaPrime, strength);
}

void GravityServer::clear() noexcept {
	_root.reset();
}

const std::string &GravityServer::getClassName() const noexcept {
	static const std::string className("GravityServer");
	return (className);
}
