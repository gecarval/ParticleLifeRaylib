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

// SceneRoot — top of the Node tree. Holds every particle in the simulation.
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

// Build a cluster of N particles randomly scattered inside a disc.
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
		Particle &p = *new Particle("P" + std::to_string(i), pos, col);
		p.setMass(mass);
		p.setRestitution(0.6f);
		p.setLockRotation(true);
		root.pushBackChild(p);
	}
}

void main2() {
	SceneRoot scene("Scene");
	spawnParticles(scene, /*N=*/1500, /*spread=*/300.0f);

	for (PhysicsBody2D *b : scene.collectBodies()) {
		const raylib::Vector2 &p = b->getPos();
		const raylib::Vector2  tangent(-p.y, p.x);
		const float			   speed = 0.5f;
		b->setLinearVel(tangent.Normalize() * speed);
	}
}

static void cameraControl(raylib::Camera2D &cam) {
	const float walkSpeed = 20.0f / cam.GetZoom();
	const float zoomDelta =
		raylib::Mouse::GetWheelMove() * cam.GetZoom() * 0.1f;
	const raylib::Vector2 mousePan = raylib::Mouse::GetDelta() / cam.GetZoom();
	static const float	  minZoom = 0.1f;
	static const float	  maxZoom = 3.0f;

	raylib::Vector2 target = cam.GetTarget();
	if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON)) {
		target -= mousePan;
	}
	if (IsKeyDown(KEY_W)) {
		const float res = target.GetY() - walkSpeed;
		target.SetY(res);
	}
	if (IsKeyDown(KEY_S)) {
		const float res = target.GetY() + walkSpeed;
		target.SetY(res);
	}
	if (IsKeyDown(KEY_A)) {
		const float res = target.GetX() - walkSpeed;
		target.SetX(res);
	}
	if (IsKeyDown(KEY_D)) {
		const float res = target.GetX() + walkSpeed;
		target.SetX(res);
	}
	cam.SetTarget(target);
	cam.SetZoom(Clamp(cam.GetZoom() + zoomDelta, minZoom, maxZoom));
}

int main(void) {
	GravityServer::getInstance();
	PhysicsServer::getInstance();
	RenderServer::getInstance();
	std::vector<Particle *>			 particles;
	std::vector<CollisionObject2D *> colliders;
	for (int i = 0; i < 1000; ++i) {
		raylib::Vector2 pos(rand() % 1000, rand() % 1000);
		Particle	   *newParticle = new Particle(std::to_string(i), pos);
		particles.push_back(newParticle);
	}
	raylib::Window		  window(1600, 900);
	const raylib::Vector2 initialPosition(window.GetSize() / 2.0f);
	raylib::Camera2D	  cam(initialPosition, initialPosition);
	// window.SetTargetFPS(60);
	GravityServer::getInstance()
		.setExpansionOrder(GravityServer::DEFAULT_EXPANSION_ORDER)
		.setMaxBodiesPerLeaf(GravityServer::DEFAULT_MAX_BODIES_PER_LEAF)
		.setTheta(GravityServer::DEFAULT_THETA);
	const float G = 200.0f;
	while (!window.ShouldClose()) {
		cameraControl(cam);
		GravityServer::getInstance().rebuild().applyGravity(G);
		PhysicsServer::getInstance().rebuild();
		for (Particle *p : particles) {
			colliders.clear();
			colliders.reserve(32);
			PhysicsServer::getInstance().getCollisions(*p, colliders);
			for (CollisionObject2D *pn2 : colliders) {
				if (*p != *pn2) {
					Particle *p2 = dynamic_cast<Particle *>(pn2);
					p->collideWith(*p2, 0.5f);
				}
			}
			p->updatePhysics();
		}
		window.BeginDrawing();
		window.ClearBackground();
		cam.BeginMode();
		RenderServer::getInstance().render(window, cam);
		cam.EndMode();
		window.DrawFPS();
		window.EndDrawing();
	}
	for (Node2D *p : particles) {
		delete p;
	}
	RenderServer::destroyInstance();
	PhysicsServer::deleteInstance();
	GravityServer::deleteInstance();
	return (0);
}
