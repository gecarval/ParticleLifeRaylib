#ifndef GRAVITYSERVER_HPP
#define GRAVITYSERVER_HPP

#include "../../include/raylib-cpp.hpp"
#include "../Object.hpp"
#include <memory>
#include <unordered_map>

class PhysicsBody2D;

// GravityServer computes N-body gravitational acceleration using the Fast
// Multipole Method (FMM): bodies are organised into an adaptive quadtree,
// each cell's mass distribution is summarised by a truncated multipole
// expansion (P2M/M2M), well-separated cell pairs exchange their influence
// through multipole-to-local (M2L) translations, local expansions are
// pushed down to children (L2L), and nearby bodies are handled by direct
// summation (P2P). This turns the naive O(n^2) all-pairs gravity loop into
// an O(n log n) pass.
class GravityServer : public Object {
  public:
	// Opaque adaptive-quadtree node type; fully defined in GravityServer.cpp
	// and used only as an implementation detail of the FMM passes.
	struct QuadNode;

  private:
	static const int   EXPANSION_ORDER; // multipole/local expansion order (P)
	static const int   MAX_BODIES_PER_LEAF; // quadtree leaf capacity
	static const int   MAX_DEPTH;			// quadtree recursion cap
	static const float THETA;				// multipole acceptance criterion

	static GravityServer *_instance;

	static std::unordered_map<unsigned long, PhysicsBody2D *> _bodies;

	std::unique_ptr<QuadNode> _root;

	GravityServer(const std::string &instanceName = "");
	GravityServer(const GravityServer &) = delete;
	GravityServer &operator=(const GravityServer &) = delete;
	~GravityServer();

  public:
	static void addBody(PhysicsBody2D *body) noexcept;
	static void removeBody(PhysicsBody2D *body) noexcept;

	static GravityServer &getInstance() noexcept;
	static void			  deleteInstance() noexcept;

	// Core API
	void rebuild();
	void applyGravity(const float strength = 9.81f);

	// Legacy
	void applyGravity(PhysicsBody2D &body, const float strength = 9.81f) const;
	raylib::Vector2 getAcceleration(PhysicsBody2D &body,
									const float	   strength = 9.81f) const;
	void			clear() noexcept;

	virtual const std::string &getClassName() const noexcept;
};

#endif // GRAVITYSERVER_HPP
