#include "GravityServer.hpp"
#include "../node/canvas_item/node2d/collision_object2d/physics_body2d/PhysicsBody2D.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>

// ===========================================================================
// Static member definitions
// ===========================================================================
const int	GravityServer::DEFAULT_EXPANSION_ORDER = 4;
const int	GravityServer::DEFAULT_MAX_BODIES_PER_LEAF = 16;
const float GravityServer::DEFAULT_THETA = 0.5f;
const int	GravityServer::MAX_TREE_DEPTH = 24;

GravityServer *GravityServer::_instance = nullptr;
std::unordered_map<unsigned long, PhysicsBody2D *>
	GravityServer::_gravityBodies;

// ===========================================================================
// FMMNode
// ===========================================================================
GravityServer::FMMNode::FMMNode() noexcept
	: gx(0), gy(0), level(0), center(0.0f, 0.0f), halfSize(0.0f), radius2(0.0f),
	  isLeaf(false) {
	children.fill(nullptr);
}

GravityServer::FMMNode::~FMMNode() {
	for (FMMNode *c : children) {
		delete c;
	}
}

// ===========================================================================
// GravityServer ctor / dtor
// ===========================================================================
GravityServer::GravityServer(const std::string &instanceName)
	: Object(instanceName), _expansionOrder(DEFAULT_EXPANSION_ORDER),
	  _maxBodiesPerLeaf(DEFAULT_MAX_BODIES_PER_LEAF), _theta(DEFAULT_THETA),
	  _root(nullptr), _treeDepth(0), _domainMin(0.0f, 0.0f),
	  _domainMax(0.0f, 0.0f), _domainHalfSize(0.0f) {
	precomputePolynomials();
}

GravityServer::~GravityServer() {
	deleteTree();
}

// ===========================================================================
// Singleton API — mirrors PhysicsServer exactly.
// ===========================================================================
GravityServer &GravityServer::addGravityBody(PhysicsBody2D *body) noexcept {
	if (_instance != nullptr && body != nullptr) {
		_gravityBodies[body->getInstanceID()] = body;
	}
	return *_instance;
}

GravityServer &GravityServer::removeGravityBody(PhysicsBody2D *body) noexcept {
	if (_instance != nullptr && body != nullptr) {
		_gravityBodies.erase(body->getInstanceID());
	}
	return *_instance;
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

// ===========================================================================
// Multi-index bookkeeping
// ===========================================================================
//
// We pack (i, j) with i, j ≥ 0 and i + j ≤ order into a flat array using
// the diagonal layout:
//
//   index 0       → (0,0)
//   index 1..2    → (1,0), (0,1)
//   index 3..5    → (2,0), (1,1), (0,2)
//   ...
//   index offset(n) .. offset(n)+n → (n,0), (n-1,1), ..., (0,n)
//
// where offset(n) = n(n+1)/2 is the n-th triangular number. The index of
// (i, j) with i + j = n is offset(n) + (n - i) = offset(n) + j.
//
int GravityServer::coeffIndex(int i, int j) noexcept {
	const int n = i + j;
	return n * (n + 1) / 2 + j;
}

int GravityServer::numCoeffs(int order) noexcept {
	return (order + 1) * (order + 2) / 2;
}

// ===========================================================================
// Polynomial precomputation for the 1/|r| kernel
// ===========================================================================
//
// T_{i,j}(x, y) = ∂^(i+j) (1/√(x²+y²)) / ∂x^i ∂y^j
//              = P_{i,j}(x, y) / (x² + y²)^((2(i+j)+1)/2)
//
// where P_{i,j} is a homogeneous polynomial of degree i+j satisfying
// the recurrence
//   P_{0,0}     = 1
//   P_{i+1,j}   = ∂P_{i,j}/∂x · r² − (2(i+j)+1) · x · P_{i,j}
//   P_{i,j+1}   = ∂P_{i,j}/∂y · r² − (2(i+j)+1) · y · P_{i,j}
//
// We precompute P_{i,j} up to total order 2p because the M2L operator
// needs T_{α+β} with |α|, |β| ≤ p, i.e. |α+β| ≤ 2p.
//
void GravityServer::precomputePolynomials() {
	const int maxOrder = 2 * _expansionOrder;
	_polyTable.assign(maxOrder + 1, std::vector<Poly>(maxOrder + 1));

	// P_{0,0} = 1
	_polyTable[0][0].coeffs[{0, 0}] = 1.0;

	// Build up order by order. For each (i, j) with i+j = n we derive
	// (i+1, j) and (i, j+1) — i.e. order n+1 — using the recurrence.
	// We walk n from 0 up to maxOrder-1.
	for (int n = 0; n < maxOrder; ++n) {
		for (int i = 0; i <= n; ++i) {
			const int j = n - i;
			Poly	 &Pij = _polyTable[i][j];

			// r² = x² + y² corresponds to coefficients
			// {(2,0):1, (0,2):1}.
			// Multiply P_ij by r²: shift every (a,b) by (2,0)
			// and by (0,2), accumulate.
			Poly Pr2;
			for (const auto &kv : Pij.coeffs) {
				const int	 a = kv.first.first;
				const int	 b = kv.first.second;
				const double c = kv.second;
				Pr2.coeffs[{a + 2, b}] += c;
				Pr2.coeffs[{a, b + 2}] += c;
			}

			// ∂P_ij / ∂x: shift every (a,b) with a≥1 down by
			// one in x, multiplied by a.
			Poly Pdx;
			for (const auto &kv : Pij.coeffs) {
				const int	 a = kv.first.first;
				const int	 b = kv.first.second;
				const double c = kv.second;
				if (a >= 1) {
					Pdx.coeffs[{a - 1, b}] += c * a;
				}
			}

			// ∂P_ij / ∂y
			Poly Pdy;
			for (const auto &kv : Pij.coeffs) {
				const int	 a = kv.first.first;
				const int	 b = kv.first.second;
				const double c = kv.second;
				if (b >= 1) {
					Pdy.coeffs[{a, b - 1}] += c * b;
				}
			}

			const double factor = 2.0 * n + 1.0; // (2(i+j)+1) with i+j = n

			// P_{i+1, j} = ∂P_ij/∂x · r² − factor · x · P_ij
			// First term: Pdx · r² (convolve again with r²).
			Poly &Pip1j = _polyTable[i + 1][j];
			for (const auto &kv : Pdx.coeffs) {
				const int	 a = kv.first.first;
				const int	 b = kv.first.second;
				const double c = kv.second;
				Pip1j.coeffs[{a + 2, b}] += c;
				Pip1j.coeffs[{a, b + 2}] += c;
			}
			// Second term: −factor · x · P_ij  → shift (a,b) by (1,0).
			for (const auto &kv : Pij.coeffs) {
				const int	 a = kv.first.first;
				const int	 b = kv.first.second;
				const double c = kv.second;
				Pip1j.coeffs[{a + 1, b}] -= factor * c;
			}

			// P_{i, j+1} = ∂P_ij/∂y · r² − factor · y · P_ij
			Poly &Pij1 = _polyTable[i][j + 1];
			for (const auto &kv : Pdy.coeffs) {
				const int	 a = kv.first.first;
				const int	 b = kv.first.second;
				const double c = kv.second;
				Pij1.coeffs[{a + 2, b}] += c;
				Pij1.coeffs[{a, b + 2}] += c;
			}
			for (const auto &kv : Pij.coeffs) {
				const int	 a = kv.first.first;
				const int	 b = kv.first.second;
				const double c = kv.second;
				Pij1.coeffs[{a, b + 1}] -= factor * c;
			}
		} // inner for (i)
	} // outer for (n)
}

double GravityServer::evalT(double x, double y, int i, int j) const noexcept {
	const double r2 = x * x + y * y;
	if (r2 == 0.0) {
		// Singular; callers must avoid this. Return 0 as a defensive
		// default so a stray call doesn't NaN the whole simulation.
		return 0.0;
	}
	const Poly &P = _polyTable[i][j];
	double		num = 0.0;
	// Tiny optimization: accumulate the polynomial with Horner-ish
	// grouping by power of x to reduce multiplications. The polynomial
	// is small (degree i+j ≤ 2p), so a straightforward evaluation is
	// fine.
	for (const auto &kv : P.coeffs) {
		const int	 a = kv.first.first;
		const int	 b = kv.first.second;
		const double c = kv.second;
		double		 term = c;
		for (int k = 0; k < a; ++k) term *= x;
		for (int k = 0; k < b; ++k) term *= y;
		num += term;
	}
	const int	 n = i + j;
	const double r = std::sqrt(r2);
	double		 rPower = 1.0;
	for (int k = 0; k < 2 * n + 1; ++k) rPower *= r;
	return num / rPower;
}

// ===========================================================================
// Tree construction
// ===========================================================================
//
// We build a *uniform-depth* quadtree: every leaf sits at exactly
// `_treeDepth`, so cells at the same level can be enumerated purely from
// their (gx, gy) grid coordinates. The depth is chosen so the average
// leaf population is at most `_maxBodiesPerLeaf`:
//
//   4^depth · maxBodiesPerLeaf ≥ N  ⇒  depth ≥ log4(N / maxBodiesPerLeaf)
//
// We also cap depth at MAX_TREE_DEPTH so a degenerate input (e.g. many
// coincident bodies) can't blow up the tree.
//
GravityServer::FMMNode *GravityServer::buildTree() {
	deleteTree();

	const std::size_t n = _gravityBodies.size();
	if (n == 0) {
		return nullptr;
	}

	// ---- Compute axis-aligned bounding box ---------------------------
	// Start with the first body's position and expand from there.
	// We add a small epsilon so the bounding box is non-degenerate
	// when every body sits at the same point.
	auto it = _gravityBodies.begin();
	_domainMin = it->second->getPos();
	_domainMax = it->second->getPos();
	for (++it; it != _gravityBodies.end(); ++it) {
		const raylib::Vector2 &p = it->second->getPos();
		_domainMin.x = std::min(_domainMin.x, p.x);
		_domainMin.y = std::min(_domainMin.y, p.y);
		_domainMax.x = std::max(_domainMax.x, p.x);
		_domainMax.y = std::max(_domainMax.y, p.y);
	}

	// Make the domain square and add a tiny margin so bodies on the
	// boundary fall *inside* cells rather than on the right/top edge.
	const float dx = _domainMax.x - _domainMin.x;
	const float dy = _domainMax.y - _domainMin.y;
	float		side = std::max(dx, dy);
	if (side < 1e-6f) side = 1e-6f; // all bodies coincident
	const float margin = side * 1e-4f;
	_domainMin.x -= margin;
	_domainMin.y -= margin;
	side += 2.0f * margin;
	_domainMax.x = _domainMin.x + side;
	_domainMax.y = _domainMin.y + side;
	_domainHalfSize = 0.5f * side;

	// ---- Choose tree depth -------------------------------------------
	int depth = 0;
	if (n > static_cast<std::size_t>(_maxBodiesPerLeaf)) {
		// depth = ceil(log4(n / maxBodiesPerLeaf))
		//       = ceil(0.5 * log2(n / maxBodiesPerLeaf))
		const double ratio =
			static_cast<double>(n) / static_cast<double>(_maxBodiesPerLeaf);
		depth = static_cast<int>(std::ceil(0.5 * std::log2(ratio)));
	}
	depth = std::min(depth, MAX_TREE_DEPTH);
	depth = std::max(depth, 0);
	_treeDepth = depth;

	// ---- Bucket every body into the root, then recurse ---------------
	std::vector<PhysicsBody2D *> allBodies;
	allBodies.reserve(n);
	for (const auto &kv : _gravityBodies) {
		allBodies.push_back(kv.second);
	}

	const raylib::Vector2 rootCenter((_domainMin.x + _domainMax.x) * 0.5f,
									 (_domainMin.y + _domainMax.y) * 0.5f);
	_root = buildNodeRecursive(0, 0, 0, rootCenter, _domainHalfSize, allBodies);
	return _root;
}

GravityServer::FMMNode *
GravityServer::buildNodeRecursive(int gx, int gy, int level,
								  const raylib::Vector2 &center, float halfSize,
								  std::vector<PhysicsBody2D *> &bodies) {
	FMMNode *node = new FMMNode();
	node->gx = gx;
	node->gy = gy;
	node->level = level;
	node->center = center;
	node->halfSize = halfSize;
	const float r = halfSize * static_cast<float>(std::sqrt(2.0));
	node->radius2 = r * r;

	const int numCoeff = numCoeffs(_expansionOrder);
	node->multipole.assign(numCoeff, 0.0);
	node->local.assign(numCoeff, 0.0);

	if (level >= _treeDepth) {
		// Leaf — keep the bodies that fall inside this cell.
		node->isLeaf = true;
		node->bodies.swap(bodies);
		return node;
	}

	// Internal node — partition bodies into the four quadrants and
	// recurse. Empty children are still materialised so the tree is
	// a perfect 4-ary tree (uniform depth).
	node->isLeaf = false;
	const float			  q = halfSize * 0.5f;
	const raylib::Vector2 childCenters[4] = {
		raylib::Vector2(center.x - q, center.y - q), // (0,0) → SW
		raylib::Vector2(center.x + q, center.y - q), // (1,0) → SE
		raylib::Vector2(center.x - q, center.y + q), // (0,1) → NW
		raylib::Vector2(center.x + q, center.y + q), // (1,1) → NE
	};

	std::vector<PhysicsBody2D *> childBodies[4];
	for (PhysicsBody2D *b : bodies) {
		const raylib::Vector2 &p = b->getPos();
		const int			   ix = (p.x >= center.x) ? 1 : 0;
		const int			   iy = (p.y >= center.y) ? 1 : 0;
		const int			   idx = (iy << 1) | ix;
		childBodies[idx].push_back(b);
	}

	// Recurse. The child grid coordinates are (2gx + ix, 2gy + iy).
	for (int iy = 0; iy < 2; ++iy) {
		for (int ix = 0; ix < 2; ++ix) {
			const int idx = (iy << 1) | ix;
			node->children[idx] =
				buildNodeRecursive(2 * gx + ix, 2 * gy + iy, level + 1,
								   childCenters[idx], q, childBodies[idx]);
		}
	}
	return node;
}

void GravityServer::deleteTree() noexcept {
	delete _root;
	_root = nullptr;
	_treeDepth = 0;
	_domainHalfSize = 0.0f;
}

// ===========================================================================
// FMM operators
// ===========================================================================

// ---- P2M: Particle → Multipole --------------------------------------------
//
// M_α(c) = Σ_particles m_p · (p − c)^α
//
void GravityServer::p2m(FMMNode *node) noexcept {
	if (!node->isLeaf) return;
	const int p = _expansionOrder;
	for (PhysicsBody2D *b : node->bodies) {
		const double dx = static_cast<double>(b->getPos().x) -
						  static_cast<double>(node->center.x);
		const double dy = static_cast<double>(b->getPos().y) -
						  static_cast<double>(node->center.y);
		const double m = static_cast<double>(b->getMass());

		// Precompute powers of dx, dy up to p.
		std::vector<double> px(p + 1), py(p + 1);
		px[0] = 1.0;
		py[0] = 1.0;
		for (int k = 1; k <= p; ++k) {
			px[k] = px[k - 1] * dx;
			py[k] = py[k - 1] * dy;
		}

		for (int i = 0; i <= p; ++i) {
			for (int j = 0; j <= p - i; ++j) {
				node->multipole[coeffIndex(i, j)] += m * px[i] * py[j];
			}
		}
	}
}

// ---- M2M: child multipole → parent multipole ------------------------------
//
// If the child's centre is at displacement d = child.center − parent.center
// from the parent's centre, the parent's multipole aggregates as
//
//   M_parent[α] = Σ_{β ≤ α} C(α, β) · d^(α−β) · M_child[β]
//
// where C(α, β) = (α_i choose β_i) · (α_j choose β_j) is the multi-index
// binomial coefficient.
//
void GravityServer::m2m(FMMNode *parent) noexcept {
	const int p = _expansionOrder;

	// Precompute binomial coefficients up to p.
	// binom[n][k] = n! / (k! (n-k)!)
	static thread_local int	 binom[16][16];
	static thread_local bool init = false;
	if (!init) {
		for (int n = 0; n < 16; ++n) {
			binom[n][0] = 1;
			for (int k = 1; k <= n; ++k) {
				binom[n][k] =
					binom[n - 1][k - 1] + (k <= n - 1 ? binom[n - 1][k] : 0);
			}
		}
		init = true;
	}

	for (const FMMNode *child : parent->children) {
		if (child == nullptr) continue;
		const double dx = static_cast<double>(child->center.x) -
						  static_cast<double>(parent->center.x);
		const double dy = static_cast<double>(child->center.y) -
						  static_cast<double>(parent->center.y);

		std::vector<double> px(p + 1), py(p + 1);
		px[0] = 1.0;
		py[0] = 1.0;
		for (int k = 1; k <= p; ++k) {
			px[k] = px[k - 1] * dx;
			py[k] = py[k - 1] * dy;
		}

		for (int ai = 0; ai <= p; ++ai) {
			for (int aj = 0; aj <= p - ai; ++aj) {
				double sum = 0.0;
				for (int bi = 0; bi <= ai; ++bi) {
					for (int bj = 0; bj <= aj; ++bj) {
						const double c = static_cast<double>(binom[ai][bi]) *
										 static_cast<double>(binom[aj][bj]);
						sum += c * px[ai - bi] * py[aj - bj] *
							   child->multipole[coeffIndex(bi, bj)];
					}
				}
				parent->multipole[coeffIndex(ai, aj)] += sum;
			}
		}
	}
}

// ---- M2L: source multipole → target local ---------------------------------
//
//   L_target[α] += Σ_β (−1)^|β| · T_{α+β}(c_t − c_s) · M_source[β] / β!
//
// Valid when c_t and c_s are well-separated (|c_t − c_s| > 2 · cellSize),
// which is exactly what the interaction-list construction guarantees.
//
void GravityServer::m2l(const FMMNode *source, FMMNode *target) noexcept {
	const int	 p = _expansionOrder;
	const double dx = static_cast<double>(target->center.x) -
					  static_cast<double>(source->center.x);
	const double dy = static_cast<double>(target->center.y) -
					  static_cast<double>(source->center.y);

	// Precompute factorials up to p.
	static thread_local double fact[16];
	static thread_local bool   init = false;
	if (!init) {
		fact[0] = 1.0;
		for (int k = 1; k < 16; ++k) fact[k] = fact[k - 1] * k;
		init = true;
	}

	// Precompute T_{i,j}(dx, dy) for i+j <= 2p.
	// We index T by (i, j) with i+j <= 2p, stored in a 2D table sized
	// (2p+1) x (2p+1) for direct access.
	const int						 maxN = 2 * p;
	std::vector<std::vector<double>> T(maxN + 1,
									   std::vector<double>(maxN + 1, 0.0));
	for (int i = 0; i <= maxN; ++i) {
		for (int j = 0; j <= maxN - i; ++j) {
			T[i][j] = evalT(dx, dy, i, j);
		}
	}

	for (int ai = 0; ai <= p; ++ai) {
		for (int aj = 0; aj <= p - ai; ++aj) {
			double sum = 0.0;
			for (int bi = 0; bi <= p; ++bi) {
				for (int bj = 0; bj <= p - bi; ++bj) {
					const double sign = ((bi + bj) & 1) ? -1.0 : 1.0;
					const double invFact = 1.0 / (fact[bi] * fact[bj]);
					sum += sign * invFact * T[ai + bi][aj + bj] *
						   source->multipole[coeffIndex(bi, bj)];
				}
			}
			target->local[coeffIndex(ai, aj)] += sum;
		}
	}
}

// ---- L2L: parent local → child local --------------------------------------
//
//   L_child[α] = Σ_{β ≥ α} L_parent[β] · d^(β−α) / (β−α)!
//
// with d = child.center − parent.center. The naive binomial form
//   L_child[α] = Σ_{β ≥ α} C(β, α) · d^(β−α) · L_parent[β]
// is *wrong* under our convention φ(r) = Σ_α L_α · (r−c)^α / α! — the
// α! in the denominator cancels the β! in C(β, α) and leaves 1/(β−α)!.
//
void GravityServer::l2l(const FMMNode *parent, FMMNode *child) noexcept {
	const int p = _expansionOrder;

	static thread_local double fact[16];
	static thread_local bool   init = false;
	if (!init) {
		fact[0] = 1.0;
		for (int k = 1; k < 16; ++k) fact[k] = fact[k - 1] * k;
		init = true;
	}

	const double dx = static_cast<double>(child->center.x) -
					  static_cast<double>(parent->center.x);
	const double dy = static_cast<double>(child->center.y) -
					  static_cast<double>(parent->center.y);

	std::vector<double> px(p + 1), py(p + 1);
	px[0] = 1.0;
	py[0] = 1.0;
	for (int k = 1; k <= p; ++k) {
		px[k] = px[k - 1] * dx;
		py[k] = py[k - 1] * dy;
	}

	for (int ai = 0; ai <= p; ++ai) {
		for (int aj = 0; aj <= p - ai; ++aj) {
			double sum = 0.0;
			for (int bi = ai; bi <= p; ++bi) {
				for (int bj = aj; bj <= p - bi; ++bj) {
					const double invF = 1.0 / (fact[bi - ai] * fact[bj - aj]);
					sum += invF * px[bi - ai] * py[bj - aj] *
						   parent->local[coeffIndex(bi, bj)];
				}
			}
			child->local[coeffIndex(ai, aj)] = sum;
		}
	}
}

// ---- L2P: Local → Particle (apply acceleration to bodies) -----------------
//
// Potential at r in the cell:  φ(r) = Σ_α L_α · (r − c)^α / α!
// (positive — φ = +Σ G·m/|r−s|, the *negative* of the conventional
// gravitational potential V = −G·m/|r−s|).
//
// Acceleration:                a(r) = +∇_r φ(r) · strength
//   (positive gradient — because φ = −V, the conventional "a = −∇V" becomes
//   "a = +∇φ", and the result is the attractive acceleration
//   Σ_other G·m_other·(other.pos − r)/|Δ|³ that matches
//   PhysicsBody2D::applyNewtonianGravity.)
//
//   a_x = +strength · Σ_{i≥1, j≥0, i+j≤p} L_{i,j} · (r−c).x^(i−1) · (r−c).y^j /
//   ((i−1)! · j!) a_y = +strength · Σ_{i≥0, j≥1, i+j≤p} L_{i,j} · (r−c).x^i ·
//   (r−c).y^(j−1) / (i! · (j−1)!)
//
void GravityServer::l2p(FMMNode *node, double strength) noexcept {
	if (!node->isLeaf) return;
	const int p = _expansionOrder;

	static thread_local double fact[16];
	static thread_local bool   init = false;
	if (!init) {
		fact[0] = 1.0;
		for (int k = 1; k < 16; ++k) fact[k] = fact[k - 1] * k;
		init = true;
	}

	const double cx = static_cast<double>(node->center.x);
	const double cy = static_cast<double>(node->center.y);

	for (PhysicsBody2D *b : node->bodies) {
		const double dx = static_cast<double>(b->getPos().x) - cx;
		const double dy = static_cast<double>(b->getPos().y) - cy;

		std::vector<double> px(p + 1), py(p + 1);
		px[0] = 1.0;
		py[0] = 1.0;
		for (int k = 1; k <= p; ++k) {
			px[k] = px[k - 1] * dx;
			py[k] = py[k - 1] * dy;
		}

		double ax = 0.0;
		double ay = 0.0;
		for (int i = 0; i <= p; ++i) {
			for (int j = 0; j <= p - i; ++j) {
				const double L = node->local[coeffIndex(i, j)];
				if (i >= 1) {
					const double invF = 1.0 / (fact[i - 1] * fact[j]);
					ax += L * px[i - 1] * py[j] * invF;
				}
				if (j >= 1) {
					const double invF = 1.0 / (fact[i] * fact[j - 1]);
					ay += L * px[i] * py[j - 1] * invF;
				}
			}
		}
		// applyForce(force) divides by mass internally, so we
		// multiply by mass here to get the desired acceleration delta.
		const float			  m = b->getMass();
		const raylib::Vector2 force(static_cast<float>(strength * ax) * m,
									static_cast<float>(strength * ay) * m);
		b->applyForce(force);
	}
}

// ---- P2P: Particle → Particle (direct near-field) -------------------------
//
// For each body in `dst`, sum the gravitational acceleration from every
// body in `src`. When sameBucket is true the two vectors are the same
// physical bucket, so we walk unordered pairs and apply the result to
// both ends.
//
void GravityServer::p2p(const std::vector<PhysicsBody2D *> &src,
						const std::vector<PhysicsBody2D *> &dst,
						double strength, bool sameBucket) noexcept {
	if (sameBucket) {
		const std::size_t n = dst.size();
		for (std::size_t a = 0; a < n; ++a) {
			PhysicsBody2D		  *ba = dst[a];
			const raylib::Vector2 &pa = ba->getPos();
			const float			   ma = ba->getMass();
			double				   ax = 0.0;
			double				   ay = 0.0;
			for (std::size_t b = a + 1; b < n; ++b) {
				PhysicsBody2D		  *bb = dst[b];
				const raylib::Vector2 &pb = bb->getPos();
				const double		   ddx = static_cast<double>(pb.x - pa.x);
				const double		   ddy = static_cast<double>(pb.y - pa.y);
				const double		   r2 = ddx * ddx + ddy * ddy;
				if (r2 <= 0.0) continue;
				const double r = std::sqrt(r2);
				const double r3 = r2 * r;
				const double mb = static_cast<double>(bb->getMass());
				// Acceleration on ba due to bb: G·mb·(pb-pa)/r³
				const double kx = strength * mb * ddx / r3;
				const double ky = strength * mb * ddy / r3;
				ax += kx;
				ay += ky;
				// Newton's third law: bb feels −k from ba.
				const float			  mba = ma;
				const raylib::Vector2 fOnB(static_cast<float>(-kx) * mba,
										   static_cast<float>(-ky) * mba);
				bb->applyForce(fOnB);
			}
			const raylib::Vector2 fOnA(static_cast<float>(ax) * ma,
									   static_cast<float>(ay) * ma);
			ba->applyForce(fOnA);
		}
	} else {
		for (PhysicsBody2D *bd : dst) {
			const raylib::Vector2 &pd = bd->getPos();
			const float			   md = bd->getMass();
			double				   ax = 0.0;
			double				   ay = 0.0;
			for (PhysicsBody2D *bs : src) {
				if (bs == bd) continue; // defensive
				const raylib::Vector2 &ps = bs->getPos();
				const double		   ddx = static_cast<double>(ps.x - pd.x);
				const double		   ddy = static_cast<double>(ps.y - pd.y);
				const double		   r2 = ddx * ddx + ddy * ddy;
				if (r2 <= 0.0) continue;
				const double r = std::sqrt(r2);
				const double r3 = r2 * r;
				const double ms = static_cast<double>(bs->getMass());
				ax += strength * ms * ddx / r3;
				ay += strength * ms * ddy / r3;
			}
			const raylib::Vector2 force(static_cast<float>(ax) * md,
										static_cast<float>(ay) * md);
			bd->applyForce(force);
		}
	}
}

// ===========================================================================
// Tree traversal
// ===========================================================================

void GravityServer::upwardPass(FMMNode *node) noexcept {
	if (node == nullptr) return;
	if (node->isLeaf) {
		p2m(node);
		return;
	}
	for (FMMNode *c : node->children) {
		upwardPass(c);
	}
	m2m(node);
}

void GravityServer::downwardPass(FMMNode *node) noexcept {
	// Uniform-depth tree: the per-level sweep in downwardPassByLevel
	// handles all the work. This overload exists only for API
	// completeness and is intentionally a no-op.
	(void)node;
}

// ---------------------------------------------------------------------------
// Per-level downward sweep. For each level L from 1 to _treeDepth:
//   * For every cell at level L:
//       - Inherit parent's local expansion via L2L (L > 0).
//       - For each cell in this cell's interaction list (cells at level L
//         whose parents are siblings of this cell's parent but which are
//         not themselves in this cell's 3×3 neighbourhood), apply M2L.
//
// The interaction list of a cell N at (gx, gy, level L) is exactly the
// set of cells at level L whose (gx', gy') are in the 6×6 region centred
// on N's parent's children but outside N's 3×3 neighbourhood — i.e. up
// to 27 well-separated source cells.
//
// Walking every level explicitly is O(4^L) per level and O(4^D) overall,
// which matches the standard FMM complexity.
// ---------------------------------------------------------------------------
void GravityServer::downwardPassByLevel() noexcept {
	if (_root == nullptr) return;

	std::vector<FMMNode *> current, next;
	current.push_back(_root);

	for (int level = 1; level <= _treeDepth; ++level) {
		next.clear();
		const int dim = 1 << level;
		const int parentDim = 1 << (level - 1);

		// First, L2L from each parent into its children, collecting
		// the children into `next` for the M2L sweep.
		for (FMMNode *parent : current) {
			for (FMMNode *child : parent->children) {
				if (child != nullptr) {
					l2l(parent, child);
					next.push_back(child);
				}
			}
		}

		// Then, M2L against each cell's interaction list. Skip cells
		// at level < 2 — their interaction list is empty because
		// their parent's colleague range is the entire level.
		for (FMMNode *node : next) {
			if (node->level < 2) continue;
			const int parentGx = node->gx >> 1;
			const int parentGy = node->gy >> 1;

			for (int pgx = parentGx - 1; pgx <= parentGx + 1; ++pgx) {
				if (pgx < 0 || pgx >= parentDim) continue;
				for (int pgy = parentGy - 1; pgy <= parentGy + 1; ++pgy) {
					if (pgy < 0 || pgy >= parentDim) continue;

					// Enumerate the 4 children of this
					// parent-level colleague.
					for (int cy = 0; cy < 2; ++cy) {
						for (int cx = 0; cx < 2; ++cx) {
							const int cgx = 2 * pgx + cx;
							const int cgy = 2 * pgy + cy;
							// Skip N's 3×3 neighbourhood.
							if (std::abs(cgx - node->gx) <= 1 &&
								std::abs(cgy - node->gy) <= 1) {
								continue;
							}
							if (cgx < 0 || cgx >= dim || cgy < 0 ||
								cgy >= dim) {
								continue;
							}
							FMMNode *src = lookupCell(cgx, cgy, level);
							if (src != nullptr) {
								m2l(src, node);
							}
						}
					}
				}
			}
		}

		current.swap(next);
	}
}

// ---------------------------------------------------------------------------
// lookupCell: descend from the root to the cell at (gx, gy, level).
// For a uniform-depth tree, every cell along the path exists, so this
// never returns nullptr for in-range coordinates.
// ---------------------------------------------------------------------------
GravityServer::FMMNode *GravityServer::lookupCell(int gx, int gy,
												  int level) const noexcept {
	if (_root == nullptr || level < 0 || level > _treeDepth) return nullptr;
	FMMNode *node = _root;
	for (int L = 1; L <= level; ++L) {
		const int bit = level - L;
		const int ix = (gx >> bit) & 1;
		const int iy = (gy >> bit) & 1;
		const int idx = (iy << 1) | ix;
		if (node->children[idx] == nullptr) return nullptr;
		node = node->children[idx];
	}
	return node;
}

// ---------------------------------------------------------------------------
// Near-field pass: for each leaf, P2P against every body in itself and
// in the 8 adjacent leaves at the same depth.
// ---------------------------------------------------------------------------
void GravityServer::nearFieldPassRoot(double strength) noexcept {
	if (_root == nullptr) return;
	// Collect all leaves.
	std::vector<FMMNode *> leaves;
	std::vector<FMMNode *> stack;
	stack.push_back(_root);
	while (!stack.empty()) {
		FMMNode *n = stack.back();
		stack.pop_back();
		if (n->isLeaf) {
			leaves.push_back(n);
		} else {
			for (FMMNode *c : n->children) {
				if (c != nullptr) stack.push_back(c);
			}
		}
	}

	const int dim = 1 << _treeDepth;
	for (FMMNode *leaf : leaves) {
		// Self-interactions.
		p2p(leaf->bodies, leaf->bodies, strength, /*sameBucket=*/true);
		// 3×3 neighbourhood (skip self, already done).
		for (int dy = -1; dy <= 1; ++dy) {
			for (int dx = -1; dx <= 1; ++dx) {
				if (dx == 0 && dy == 0) continue;
				const int nx = leaf->gx + dx;
				const int ny = leaf->gy + dy;
				if (nx < 0 || nx >= dim || ny < 0 || ny >= dim) continue;
				FMMNode *nbr = lookupCell(nx, ny, _treeDepth);
				if (nbr == nullptr || nbr->bodies.empty()) continue;
				// Each unordered pair (leaf, nbr) is processed twice
				// (once from leaf's side, once from nbr's side). To
				// avoid double-counting we only process pairs where
				// (leaf.gx, leaf.gy) < (nbr.gx, nbr.gy) lexicographically.
				if (std::make_pair(leaf->gx, leaf->gy) <
					std::make_pair(nbr->gx, nbr->gy)) {
					p2p(nbr->bodies, leaf->bodies, strength, false);
					p2p(leaf->bodies, nbr->bodies, strength, false);
				}
			}
		}
	}
}

void GravityServer::nearFieldPass(FMMNode *node, double strength) noexcept {
	(void)node;
	(void)strength;
	// Real work is done by nearFieldPassRoot above; this stub exists
	// only to satisfy the header declaration.
}

// ===========================================================================
// Barnes-Hut fallback (used by computeAcceleration when running a per-body
// query without going through the full FMM evaluation pipeline).
// ---------------------------------------------------------------------------
// At each cell, if (cell.radius / distance) < theta, evaluate the cell's
// multipole expansion directly at the query position. Otherwise recurse
// into children (or, at a leaf, do direct P2P against every body).
// ===========================================================================
void GravityServer::bhWalk(FMMNode *node, const raylib::Vector2 &queryPos,
						   double			strength,
						   raylib::Vector2 &outAcc) const noexcept {
	if (node == nullptr) return;

	const double dx = static_cast<double>(node->center.x - queryPos.x);
	const double dy = static_cast<double>(node->center.y - queryPos.y);
	const double r2 = dx * dx + dy * dy;

	if (node->isLeaf) {
		// Direct P2P against every body in this leaf (excluding the
		// query body itself, which the caller detects by position).
		for (PhysicsBody2D *b : node->bodies) {
			const raylib::Vector2 &bp = b->getPos();
			const double		   ddx = static_cast<double>(bp.x - queryPos.x);
			const double		   ddy = static_cast<double>(bp.y - queryPos.y);
			const double		   rr2 = ddx * ddx + ddy * ddy;
			if (rr2 <= 0.0) continue;
			const double rr = std::sqrt(rr2);
			const double r3 = rr2 * rr;
			const double m = static_cast<double>(b->getMass());
			outAcc.x += static_cast<float>(strength * m * ddx / r3);
			outAcc.y += static_cast<float>(strength * m * ddy / r3);
		}
		return;
	}

	// Barnes-Hut acceptance: open the cell if it's "far enough".
	const float cellRadius =
		node->halfSize * static_cast<float>(std::sqrt(2.0));
	if (r2 > 0.0 &&
		static_cast<float>(cellRadius) / static_cast<float>(std::sqrt(r2)) <
			_theta) {
		// Evaluate the multipole expansion directly at (queryPos −
		// node.center).
		const int p = _expansionOrder;

		static thread_local double fact[16];
		static thread_local bool   init = false;
		if (!init) {
			fact[0] = 1.0;
			for (int k = 1; k < 16; ++k) fact[k] = fact[k - 1] * k;
			init = true;
		}

		// The far-field potential at r due to this cell's multipole M
		// (about c) is
		//    φ(r) = Σ_α (−1)^|α| · T_α(r − c) · M_α / α!
		// and the acceleration is −∇φ · strength.
		//
		// Rather than differentiate T_α symbolically, we evaluate
		// φ at r and at r + ε·e_x, r + ε·e_y and take finite
		// differences. This is O(p²) per evaluation — fine for a
		// per-body query, but not what we'd want inside applyGravity.
		const double eps = 1e-4 * node->halfSize;
		auto		 evalPot = [&](double qx, double qy) -> double {
			const double ddx = qx - static_cast<double>(node->center.x);
			const double ddy = qy - static_cast<double>(node->center.y);
			double		 pot = 0.0;
			for (int i = 0; i <= p; ++i) {
				for (int j = 0; j <= p - i; ++j) {
					const double sign = ((i + j) & 1) ? -1.0 : 1.0;
					const double invF = 1.0 / (fact[i] * fact[j]);
					pot += sign * invF * evalT(ddx, ddy, i, j) *
						   node->multipole[coeffIndex(i, j)];
				}
			}
			return pot;
		};

		const double qx = static_cast<double>(queryPos.x);
		const double qy = static_cast<double>(queryPos.y);
		const double p0 = evalPot(qx, qy);
		const double ppx = evalPot(qx + eps, qy);
		const double ppy = evalPot(qx, qy + eps);
		const double dphi_dx = (ppx - p0) / eps;
		const double dphi_dy = (ppy - p0) / eps;
		// φ = +G·m/|r−s| (positive), so a = +∇φ (attractive).
		outAcc.x += static_cast<float>(strength * dphi_dx);
		outAcc.y += static_cast<float>(strength * dphi_dy);
		return;
	}

	// Recurse.
	for (FMMNode *c : node->children) {
		bhWalk(c, queryPos, strength, outAcc);
	}
}

// Direct O(N) sum — used when no tree has been built yet.
void GravityServer::directSum(PhysicsBody2D &body, double strength,
							  raylib::Vector2 &outAcc) const noexcept {
	const raylib::Vector2 &p = body.getPos();
	double				   ax = 0.0;
	double				   ay = 0.0;
	for (const auto &kv : _gravityBodies) {
		PhysicsBody2D *other = kv.second;
		if (other == &body) continue;
		const raylib::Vector2 &op = other->getPos();
		const double		   dx = static_cast<double>(op.x - p.x);
		const double		   dy = static_cast<double>(op.y - p.y);
		const double		   r2 = dx * dx + dy * dy;
		if (r2 <= 0.0) continue;
		const double r = std::sqrt(r2);
		const double r3 = r2 * r;
		const double m = static_cast<double>(other->getMass());
		ax += strength * m * dx / r3;
		ay += strength * m * dy / r3;
	}
	outAcc.x += static_cast<float>(ax);
	outAcc.y += static_cast<float>(ay);
}

// ===========================================================================
// Public API
// ===========================================================================

GravityServer &GravityServer::rebuild() {
	buildTree();
	if (_root == nullptr) return *this;
	upwardPass(_root);
	downwardPassByLevel();
	// Near-field P2P is *not* done here — it's done inside applyGravity()
	// so that computeAcceleration() (which uses the Barnes-Hut fallback)
	// doesn't pay for it.
	return *this;
}

GravityServer &GravityServer::applyGravity(float strength) noexcept {
	if (_root == nullptr) {
		// Lazy rebuild if the user forgot.
		rebuild();
	}
	if (_root == nullptr) return *this; // still no bodies

	// Far field: L2P at every leaf.
	// We walk the tree and apply L2P at leaves.
	std::vector<FMMNode *> stack;
	stack.push_back(_root);
	while (!stack.empty()) {
		FMMNode *n = stack.back();
		stack.pop_back();
		if (n->isLeaf) {
			l2p(n, static_cast<double>(strength));
		} else {
			for (FMMNode *c : n->children) {
				if (c != nullptr) stack.push_back(c);
			}
		}
	}

	// Near field: P2P against adjacent leaves.
	nearFieldPassRoot(static_cast<double>(strength));
	return *this;
}

const GravityServer &
GravityServer::computeAcceleration(PhysicsBody2D &body, float strength,
								   raylib::Vector2 &outAcc) const noexcept {
	outAcc.x = 0.0f;
	outAcc.y = 0.0f;

	if (_root == nullptr) {
		directSum(body, static_cast<double>(strength), outAcc);
		return *this;
	}

	// Use Barnes-Hut walk against the (already-built) multipole tree.
	// This avoids recomputing the full downward pass for a single query.
	bhWalk(_root, body.getPos(), static_cast<double>(strength), outAcc);
	return *this;
}

raylib::Vector2
GravityServer::computeAcceleration(PhysicsBody2D &body,
								   float		  strength) const noexcept {
	raylib::Vector2 acc(0.0f, 0.0f);
	computeAcceleration(body, strength, acc);
	return acc;
}

GravityServer &GravityServer::clear() noexcept {
	deleteTree();
	return *this;
}

// ===========================================================================
// Configuration
// ===========================================================================
GravityServer &GravityServer::setExpansionOrder(int p) noexcept {
	if (p < 0) p = 0;
	if (p > 8) p = 8; // 2p ≤ 16, fits our static tables
	if (p == _expansionOrder) return *this;
	_expansionOrder = p;
	precomputePolynomials();
	// Tree is now stale; force a rebuild on next applyGravity/rebuild.
	deleteTree();
	return *this;
}

int GravityServer::getExpansionOrder() const noexcept {
	return _expansionOrder;
}

GravityServer &GravityServer::setMaxBodiesPerLeaf(int n) noexcept {
	if (n < 1) n = 1;
	_maxBodiesPerLeaf = n;
	return *this;
}

int GravityServer::getMaxBodiesPerLeaf() const noexcept {
	return _maxBodiesPerLeaf;
}

GravityServer &GravityServer::setTheta(float theta) noexcept {
	if (theta < 0.0f) theta = 0.0f;
	_theta = theta;
	return *this;
}

float GravityServer::getTheta() const noexcept {
	return _theta;
}

std::size_t GravityServer::getBodyCount() const noexcept {
	return _gravityBodies.size();
}

const std::string &GravityServer::getClassName() const noexcept {
	static const std::string className("GravityServer");
	return className;
}
