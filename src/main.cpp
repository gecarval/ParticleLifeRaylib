// ===========================================================================
// main.cpp — example main loop wiring the new GravityServer (FMM) into the
// existing PhysicsServer + Node + Particle framework.
//
// The pattern is a direct parallel of how PhysicsServer is used today:
//
//   PhysicsServer::getInstance().rebuild();      // spatial hash refresh
//   ... for each body: server.getCollisions(body, out) ...
//
// becomes, for gravity:
//
//   GravityServer::getInstance().rebuild();      // FMM tree refresh
//   GravityServer::getInstance().applyGravity(G); // push accel onto every body
//
// PhysicsBody2D self-registers with GravityServer in its ctor (mirroring
// CollisionObject2D → PhysicsServer), so you do NOT need to call
// addGravityBody / removeGravityBody yourself.
// ===========================================================================

#include "../classes/gravity_server/GravityServer.hpp"
#include "../classes/node/Node.hpp"
#include "../classes/node/canvas_item/node2d/collision_object2d/physics_body2d/particle/Particle.hpp"
#include "../classes/physics_server/PhysicsServer.hpp"
#include "../classes/render_server/RenderServer.hpp"
#include "../include/raylib-cpp.hpp"

#include <random>
#include <vector>

// ---------------------------------------------------------------------------
// SceneRoot — top of the Node tree. Holds every particle in the simulation.
// ---------------------------------------------------------------------------
class SceneRoot : public Node {
  public:
	SceneRoot(const std::string &name = "SceneRoot") : Node(name) {
	}

	// Convenience: collect every descendant that is a PhysicsBody2D so we
	// can iterate them for collision resolution and physics integration.
	std::vector<PhysicsBody2D *> collectBodies() noexcept {
		std::vector<PhysicsBody2D *> out;
		collectBodiesRecursive(this, out);
		return out;
	}

  private:
	static void
	collectBodiesRecursive(Node							*n,
						   std::vector<PhysicsBody2D *> &out) noexcept {
		PhysicsBody2D *pb = dynamic_cast<PhysicsBody2D *>(n);
		if (pb != nullptr) out.push_back(pb);
		for (Node *child : n->getChildren()) {
			collectBodiesRecursive(child, out);
		}
	}
};

// ---------------------------------------------------------------------------
// Build a cluster of N particles randomly scattered inside a disc.
// ---------------------------------------------------------------------------
static void spawnParticles(SceneRoot &root, int N, float spread) {
	std::mt19937						  rng(42);
	std::uniform_real_distribution<float> distPos(-spread, spread);
	std::uniform_real_distribution<float> distMass(0.5f, 3.0f);

	for (int i = 0; i < N; ++i) {
		const raylib::Vector2 pos(distPos(rng), distPos(rng));
		const float			  mass = distMass(rng);
		const raylib::Color	  col(
			  static_cast<unsigned char>(180 + (i % 60)),
			  static_cast<unsigned char>(180 + ((i * 7) % 60)),
			  static_cast<unsigned char>(220 - (i % 80)), 255);

		// Particle auto-registers with BOTH PhysicsServer (via
		// CollisionObject2D's ctor) and GravityServer (via PhysicsBody2D's
		// ctor). We just own its lifetime through the Node tree.
		Particle &p = *new Particle("P" + std::to_string(i), pos, col);
		p.setMass(mass);
		p.setRestitution(0.6f);
		p.setLockRotation(true);
		root.pushBackChild(p);
	}
}

// ===========================================================================
// main
// ===========================================================================
int main() {
	// --- Window / camera --------------------------------------------------
	raylib::Window window(1280, 800, "GravityServer (FMM) demo");
	window.SetTargetFPS(60);

	raylib::Camera2D camera;
	camera.SetTarget(raylib::Vector2(0.0f, 0.0f));
	camera.SetOffset(raylib::Vector2(640.0f, 400.0f));
	camera.SetZoom(1.5f);

	// --- Scene ------------------------------------------------------------
	RenderServer &renderServer = RenderServer::getInstance();
	SceneRoot	  scene("Scene");
	spawnParticles(scene, /*N=*/1500, /*spread=*/300.0f);

	// Give every particle a small random initial velocity so the cluster
	// doesn't collapse instantly into the centre of mass.
	for (PhysicsBody2D *b : scene.collectBodies()) {
		const raylib::Vector2 &p = b->getPos();
		// Tangential push around the origin → nice swirling motion.
		const raylib::Vector2 tangent(-p.y, p.x);
		const float			  speed = 0.5f;
		b->setLinearVel(tangent.Normalize() * speed);
	}

	// --- GravityServer configuration -------------------------------------
	// Default expansion order p=4 is fine for visual demos; bump to 6–8 if
	// you need higher accuracy. maxBodiesPerLeaf trades tree depth for
	// near-field bucket size; theta is only used by the per-body Barnes-
	// Hut fallback inside computeAcceleration().
	GravityServer::getInstance()
		.setExpansionOrder(GravityServer::DEFAULT_EXPANSION_ORDER)
		.setMaxBodiesPerLeaf(GravityServer::DEFAULT_MAX_BODIES_PER_LEAF)
		.setTheta(GravityServer::DEFAULT_THETA);

	const float G = 50.0f; // gravitational constant for this demo

	// --- Main loop --------------------------------------------------------
	while (!window.ShouldClose()) {
		// 1. Spatial structure refresh. Both servers expect this every
		//    frame after bodies have moved (PhysicsBody2D::updatePhysics
		//    moves them in step 4).
		PhysicsServer::getInstance().rebuild();
		GravityServer::getInstance().rebuild();

		// 2. Push gravitational acceleration onto every registered body.
		//    This is the FMM entry point — O(N) regardless of how many
		//    bodies are in the scene. Each body's _linearAcc gets
		//    incremented by Σ_other G·m_other·(other.pos − this.pos)/|Δ|³.
		GravityServer::getInstance().applyGravity(G);

		// 3. Collision response. For each body, ask PhysicsServer for its
		//    3×3 neighbourhood and resolve contact impulses. This is the
		//    existing PhysicsServer usage, unchanged.
		std::vector<PhysicsBody2D *> bodies = scene.collectBodies();
		for (PhysicsBody2D *b : bodies) {
			std::vector<CollisionObject2D *> neighbors;
			neighbors.reserve(32);
			PhysicsServer::getInstance().getCollisions(*b, neighbors);
			for (CollisionObject2D *n : neighbors) {
				b->collideWith(dynamic_cast<PhysicsBody2D &>(*n), 0.5f);
			}
		}

		// 4. Integrate motion. Reads the acceleration that applyGravity
		//    wrote into _linearAcc and integrates it into _linearVel and
		//    _pos.
		for (PhysicsBody2D *b : bodies) {
			b->updatePhysics();
		}

		// 5. Optional: query the acceleration on a single body (e.g. for
		//    a HUD readout). This uses the Barnes-Hut walk against the
		//    multipole tree and does NOT modify the body's state.
		//    Direct analogue of PhysicsServer::getCollisions(body).
		if (!bodies.empty()) {
			raylib::Vector2 a =
				GravityServer::getInstance().computeAcceleration(*bodies[0], G);
			(void)a; // could be drawn as a vector arrow on the first body
		}

		// 6. Render.
		window.BeginDrawing();
		window.ClearBackground(raylib::Color::Black());
		{
			camera.BeginMode();
			renderServer.render(window);
			camera.EndMode();
			window.DrawFPS();
		}
		window.EndDrawing();
	}

	// --- Shutdown ---------------------------------------------------------
	// The Node tree owns every Particle; destroying the tree runs each
	// Particle's destructor, which calls GravityServer::removeGravityBody
	// and PhysicsServer::removeCollisionObject automatically.
	//
	// deleteInstance() frees the servers themselves. Order matters: do it
	// AFTER the scene is gone so no body's dtor touches a dead server.
	PhysicsServer::deleteInstance();
	GravityServer::deleteInstance();
	return 0;
}
