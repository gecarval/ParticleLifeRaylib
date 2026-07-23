#include "../classes/gravity_server/GravityServer.hpp"
#include "../classes/node/canvas_item/node2d/collision_object2d/CollisionObject2D.hpp"
#include "../classes/node/canvas_item/node2d/collision_object2d/physics_body2d/particle/Particle.hpp"
#include "../classes/physics_server/PhysicsServer.hpp"
#include "../classes/render_server/RenderServer.hpp"
#include "../include/raylib-cpp.hpp"
#include <random>
#include <vector>

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

// Build a cluster of N particles randomly scattered inside a disc.
static void spawnParticles(std::vector<Particle *> &bodies, int N,
						   const float spread, const float speed = 5.0f,
						   const float minMass = 0.5f,
						   const float maxMass = 3.0f) {
	std::mt19937						  rng(42);
	std::uniform_real_distribution<float> distPos(-spread, spread);
	const raylib::Vector2				  zero(0);
	for (int i = 0; i < N; ++i) {
		const raylib::Color col(
			static_cast<unsigned char>(180 + (i % 60)),
			static_cast<unsigned char>(180 + ((i * 7) % 60)),
			static_cast<unsigned char>(220 - (i % 80)), 255);
		const raylib::Vector2 pos(distPos(rng), distPos(rng));
		const raylib::Vector2 tangent(-pos.y, pos.x);
		const float			  mass =
			Remap(pos.Distance(zero), 0, spread, maxMass, minMass);
		Particle *p = new Particle("P" + std::to_string(i), pos, col);
		p->setMass(mass);
		p->setLockRotation(true);
		p->setLinearVel(tangent.Normalize() * speed);
		bodies.push_back(p);
	}
}

int main(void) {
	std::vector<Particle *>			 particles;
	std::vector<CollisionObject2D *> colliders;
	spawnParticles(particles, 2000, 1000.0f, 5.0f, 0.5f, 3.0f);
	raylib::Window		  window(1600, 900);
	const raylib::Vector2 initialPosition(window.GetSize() / 2.0f);
	raylib::Camera2D	  cam(initialPosition, initialPosition);
	// window.SetTargetFPS(60);
	while (!window.ShouldClose()) {
		cameraControl(cam);
		window.BeginDrawing();
		window.ClearBackground();
		cam.BeginMode();
		PhysicsServer::getInstance().rebuild();
		GravityServer::getInstance().rebuild();
		GravityServer::getInstance().applyGravity(10.0f);
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
	RenderServer::deleteInstance();
	PhysicsServer::deleteInstance();
	GravityServer::deleteInstance();
	return (0);
}
