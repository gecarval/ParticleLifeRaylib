#ifndef GRAVITYSERVER_HPP
#define GRAVITYSERVER_HPP

#include "../Object.hpp"
#include "../Vector2i.hpp"
#include <array>
#include <map>
#include <unordered_map>
#include <vector>

// Forward declaration — GravityServer holds non-owning pointers to
// PhysicsBody2D instances. Including the full header here would create a
// circular include (PhysicsBody2D.cpp includes GravityServer.hpp for
// self-registration), so we keep the type incomplete in the header and include
// it only in the .cpp.
class PhysicsBody2D;

// ===========================================================================
// GravityServer
// ---------------------------------------------------------------------------
// Computes pairwise Newtonian gravitational accelerations between every
// registered PhysicsBody2D using the Fast Multipole Method (FMM).
//
// The naive approach is O(N²) — for each body, sum 1/r² contributions from
// every other body. FMM reorganises the computation through a quadtree:
//
//   * Particles in each leaf cell are aggregated into a Cartesian Taylor
//     expansion of the 1/|r-s| kernel about the cell centre (P2M).
//   * Each parent combines its four children's expansions into its own
//     (M2M), so the root holds the multipole of the entire domain.
//   * For every cell, the multipole expansions of *well-separated* cells
//     are converted into a single local Taylor expansion about that cell's
//     centre (M2L). The local expansion captures the far-field gravity.
//   * Local expansions are pushed down the tree (L2L) so every leaf ends
//     up with the full far-field expansion valid at any point inside it.
//   * Each particle finally evaluates its leaf's local expansion (L2P) for
//     the far field and adds direct particle-particle contributions (P2P)
//     for the small number of particles in nearby leaves.
//
// Result: O(N) total work for a fixed expansion order p, with error that
// falls off as (R/d)^(p+1) where R/d is the cell-size-to-distance ratio.
//
// API design mirrors PhysicsServer exactly:
//   * Singleton accessor + static register/unregister hooks called from
//     PhysicsBody2D's ctor/dtor.
//   * rebuild()              -> rebuild the spatial structure.
//   * computeAcceleration() -> per-body query (analogous to getCollisions).
//   * applyGravity()         -> apply accelerations to every registered body
//                               in one sweep (the typical per-frame call).
//   * clear()                -> drop the spatial structure.
// ===========================================================================
class GravityServer : public Object {
  public:
	// ---------------------------------------------------------------
	// Public configuration constants. They live on the class so that
	// callers can reference them in code that wants to mirror the
	// PhysicsServer::CELL_SIZE pattern.
	// ---------------------------------------------------------------
	static const int   DEFAULT_EXPANSION_ORDER;		// p  = 4
	static const int   DEFAULT_MAX_BODIES_PER_LEAF; // 16
	static const float DEFAULT_THETA;				// 0.5f
	static const int   MAX_TREE_DEPTH;				// 24

  private:
	// ---------------------------------------------------------------
	// Singleton state — same shape as PhysicsServer.
	// ---------------------------------------------------------------
	static GravityServer *_instance;

	// Master registry of bodies that should participate in gravity.
	// Keyed by Object::getInstanceID() so lookups during unregister
	// are O(1).
	static std::unordered_map<unsigned long, PhysicsBody2D *> _gravityBodies;

	// ---------------------------------------------------------------
	// FMM quadtree node.
	//
	// The tree is *uniform-depth*: every leaf lives at exactly
	// `_treeDepth`, so grid coordinates (gx, gy) together with the
	// level uniquely identify a cell and let us enumerate the 3×3
	// neighbourhood and the level-L interaction list without parent
	// pointers. A node owns its four children (heap-allocated) but
	// does *not* own the PhysicsBody2D pointers in `bodies` — those
	// are borrowed from the global registry.
	// ---------------------------------------------------------------
	struct FMMNode {
		int				gx;		  // grid x at this level (0 .. 2^level-1)
		int				gy;		  // grid y at this level
		int				level;	  // 0 at root, _treeDepth at leaves
		raylib::Vector2 center;	  // geometric centre of the cell
		float			halfSize; // half the cell's edge length
		float			radius2;  // (halfSize * sqrt(2))² — far-field check
		bool			isLeaf;
		std::array<FMMNode *, 4> children;

		// Particle pointers, populated only when isLeaf == true.
		std::vector<PhysicsBody2D *> bodies;

		// Taylor coefficients. Indexed by (i,j) with i,j >= 0 and
		// i+j <= p, flattened via coeffIndex(i,j).
		//   multipole[i,j] = Σ_particles m * (pos - center)^(i,j)
		//   local[i,j]     = coefficients of (r - center)^(i,j) / (i,j)!
		//                    in the local Taylor expansion of the
		//                    gravitational potential.
		std::vector<double> multipole;
		std::vector<double> local;

		FMMNode() noexcept;
		~FMMNode();
	};

	// ---------------------------------------------------------------
	// Configuration — exposed via setters below.
	// ---------------------------------------------------------------
	int	  _expansionOrder; // p
	int	  _maxBodiesPerLeaf;
	float _theta; // Barnes-Hut opening angle (used by
				  // computeAcceleration's fallback path).

	// ---------------------------------------------------------------
	// Precomputed symbolic polynomials P_{i,j}(x,y) such that
	//   T_{i,j}(x,y) = ∂^(i+j) (1/|r|) / ∂x^i ∂y^j = P_{i,j}(x,y) /
	//   |r|^(2(i+j)+1)
	//
	// We need T_{i,j} for i+j <= 2p because the M2L operator pairs a
	// multipole of order p with a local of order p, producing indices
	// up to 2p. The polynomials are precomputed once via the recurrence
	//   P_{0,0}     = 1
	//   P_{i+1,j}   = ∂P_{i,j}/∂x · r² − (2(i+j)+1) · x · P_{i,j}
	//   P_{i,j+1}   = ∂P_{i,j}/∂y · r² − (2(i+j)+1) · y · P_{i,j}
	// and stored as sparse coefficient tables.
	// ---------------------------------------------------------------
	struct Poly {
		// (a,b) → coefficient of x^a · y^b in P_{i,j}.
		std::map<std::pair<int, int>, double> coeffs;
	};
	std::vector<std::vector<Poly>> _polyTable; // [i][j], i+j <= 2p

	// ---------------------------------------------------------------
	// Root of the FMM tree (nullptr when nothing has been built).
	// ---------------------------------------------------------------
	FMMNode *_root;
	int		 _treeDepth; // 0 means single-cell tree (P2P only)

	// Bounding box of the most recent build, kept so callers can
	// introspect the domain if they want.
	raylib::Vector2 _domainMin;
	raylib::Vector2 _domainMax;
	float			_domainHalfSize; // half-width of the (square) root cell

	// ---------------------------------------------------------------
	// Private constructors — singleton pattern, mirroring PhysicsServer.
	// ---------------------------------------------------------------
	GravityServer(const std::string &instanceName = "");
	GravityServer(const GravityServer &) = delete;
	GravityServer &operator=(const GravityServer &) = delete;
	~GravityServer();

	// ---------------------------------------------------------------
	// Internal helpers — see GravityServer.cpp for documentation.
	// ---------------------------------------------------------------

	// Flat-array index for (i,j) with i+j <= order. The layout is
	// diagonal-by-diagonal: (0,0), (1,0), (0,1), (2,0), (1,1), (0,2), ...
	// offset(n) = n(n+1)/2 is the index of the first (i,j) with i+j=n.
	static int coeffIndex(int i, int j) noexcept;
	static int numCoeffs(int order) noexcept;

	// Build the symbolic P_{i,j} tables up to total order 2p.
	void precomputePolynomials();

	// Evaluate T_{i,j}(x,y) = P_{i,j}(x,y) / r^(2(i+j)+1) at a point.
	// Caller must guarantee (x,y) ≠ (0,0) when (i,j) ≠ (0,0) — the
	// FMM traversal never calls evalT at the origin.
	double evalT(double x, double y, int i, int j) const noexcept;

	// ---- Tree operators -------------------------------------------------

	// P2M: fill node->multipole from node->bodies.
	void p2m(FMMNode *node) noexcept;

	// M2M: combine the four children's multipoles into the parent's.
	void m2m(FMMNode *parent) noexcept;

	// M2L: convert source's multipole into a contribution added to
	//      target's local expansion.
	void m2l(const FMMNode *source, FMMNode *target) noexcept;

	// L2L: push parent's local expansion down into one of its children.
	void l2l(const FMMNode *parent, FMMNode *child) noexcept;

	// L2P: evaluate node's local expansion at every body in node->bodies
	//      and add the resulting acceleration (scaled by `strength`)
	//      onto each body's linear acceleration.
	void l2p(FMMNode *node, double strength) noexcept;

	// P2P: direct O(n*m) particle-particle acceleration between two
	//      buckets. If `sameBucket` is true, each unordered pair is
	//      processed once and the result is applied to *both* bodies;
	//      otherwise every (src, dst) ordered pair contributes to dst.
	void p2p(const std::vector<PhysicsBody2D *> &src,
			 const std::vector<PhysicsBody2D *> &dst, double strength,
			 bool sameBucket) noexcept;

	// ---- Tree traversal -------------------------------------------------

	// Recursive upward pass: post-order M2M aggregation.
	void upwardPass(FMMNode *node) noexcept;

	// Recursive downward pass stub (the real work is done by
	// downwardPassByLevel for simplicity, since the tree is uniform-
	// depth and we can enumerate levels directly).
	void downwardPass(FMMNode *node) noexcept;

	// Per-level downward sweep: for each level L ≥ 1, first L2L every
	// parent's local into its children, then M2L each cell against
	// its interaction list (cells at level L whose parents are
	// adjacent to this cell's parent but which are not themselves
	// adjacent to this cell).
	void downwardPassByLevel() noexcept;

	// Near-field P2P pass: for every leaf, sum direct interactions
	// against all bodies in itself and in the 3×3 neighbourhood at
	// the same depth.
	void nearFieldPass(FMMNode *node, double strength) noexcept;

	// Walk the tree and call nearFieldPass on every leaf.
	void nearFieldPassRoot(double strength) noexcept;

	// Descend from the root to the cell at (gx, gy, level). Returns
	// nullptr if the coordinates are out of range or the path doesn't
	// exist (the latter cannot happen for a uniform-depth tree).
	FMMNode *lookupCell(int gx, int gy, int level) const noexcept;

	// Free the entire tree.
	void deleteTree() noexcept;

	// Build a fresh uniform-depth quadtree from the current registry.
	// Returns the root, or nullptr if the registry is empty. Picks the
	// depth so the average leaf population is ≤ _maxBodiesPerLeaf.
	FMMNode *buildTree();

	// Recursive subdivision helper used by buildTree(). Materialises
	// every cell down to `_treeDepth`.
	FMMNode *buildNodeRecursive(int gx, int gy, int level,
								const raylib::Vector2 &center, float halfSize,
								std::vector<PhysicsBody2D *> &bodies);

	// Recursive helper for computeAcceleration(): Barnes-Hut-style
	// walk that uses the FMM multipole of any cell that satisfies
	// (cell.radius / distance) < theta, and recurses otherwise. Used
	// as a per-body query path that doesn't require the downward pass
	// to have run.
	void bhWalk(FMMNode *node, const raylib::Vector2 &queryPos, double strength,
				raylib::Vector2 &outAcc) const noexcept;

	// Direct O(N) sum of gravity on a single body, used as a fallback
	// when no tree has been built (e.g. computeAcceleration() called
	// before rebuild()).
	void directSum(PhysicsBody2D &body, double strength,
				   raylib::Vector2 &outAcc) const noexcept;

  public:
	// ---------------------------------------------------------------
	// Singleton API — identical in shape to PhysicsServer.
	// ---------------------------------------------------------------
	static GravityServer &addGravityBody(PhysicsBody2D *body) noexcept;
	static GravityServer &removeGravityBody(PhysicsBody2D *body) noexcept;

	static GravityServer &getInstance() noexcept;
	static void			  deleteInstance() noexcept;

	// ---------------------------------------------------------------
	// Core API.
	// ---------------------------------------------------------------

	// Rebuild the FMM tree from the current registry. Call this once
	// per frame after bodies have moved (before applyGravity() or
	// computeAcceleration()). Cheap to call when nothing has changed
	// because the previous tree is freed first.
	GravityServer &rebuild();

	// Apply FMM-computed gravitational acceleration to every registered
	// body. `strength` plays the same role as in
	// PhysicsBody2D::applyNewtonianGravity — it's the G factor that the
	// caller bakes in (each source body's own mass is also multiplied
	// in, so pass G here, not G·m).
	//
	// After this returns, every body's linear acceleration has been
	// incremented by Σ_other G·m_other·(other.pos − this.pos)/|Δ|³.
	GravityServer &applyGravity(float strength = 9.81f) noexcept;

	// Compute the gravitational acceleration vector on a single body
	// without modifying its state. The body must be registered (its
	// position is read directly, so stale trees are tolerated — but
	// for accurate far-field you should rebuild() first).
	//
	// This is the gravitational analog of PhysicsServer::getCollisions.
	const GravityServer &
	computeAcceleration(PhysicsBody2D &body, float strength,
						raylib::Vector2 &outAcc) const noexcept;

	// Convenience overload returning the acceleration by value.
	raylib::Vector2 computeAcceleration(PhysicsBody2D &body,
										float strength = 9.81f) const noexcept;

	// Drop the FMM tree but keep the registry intact.
	GravityServer &clear() noexcept;

	// ---------------------------------------------------------------
	// Configuration.
	// ---------------------------------------------------------------
	GravityServer &setExpansionOrder(int p) noexcept;
	int			   getExpansionOrder() const noexcept;

	GravityServer &setMaxBodiesPerLeaf(int n) noexcept;
	int			   getMaxBodiesPerLeaf() const noexcept;

	GravityServer &setTheta(float theta) noexcept;
	float		   getTheta() const noexcept;

	// Number of bodies currently registered.
	std::size_t getBodyCount() const noexcept;

	// Object override.
	virtual const std::string &getClassName() const noexcept override;
};

#endif // GRAVITYSERVER_HPP
