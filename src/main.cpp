#include "../classes/node/canvas_item/node2d/collision_object2d/CollisionObject2D.hpp"
#include "../classes/node/canvas_item/node2d/collision_object2d/physics_body2d/particle/Particle.hpp"
#include "../classes/physics_server/PhysicsServer.hpp"
#include "../classes/render_server/RenderServer.hpp"
#include <memory>
#include <string>

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
	PhysicsServer		   &physicsServer = PhysicsServer::getInstance();
	RenderServer		   &renderServer = RenderServer::getInstance();
	std::vector<Particle *> particles;
	std::vector<CollisionObject2D *> colliders;
	colliders.reserve(1000);
	particles.reserve(1000);
	for (int i = 0; i < 1000; ++i) {
		raylib::Vector2 pos(rand() % 800, rand() % 600);
		Particle	   *newParticle = new Particle(std::to_string(i), pos);
		newParticle->setVisibleDebug(true);
		particles.push_back(newParticle);
	}
	raylib::Window		  window(800, 600);
	const raylib::Vector2 initialPosition(window.GetSize() / 2.0f);
	raylib::Camera2D	  cam(initialPosition, initialPosition);
	// window.SetTargetFPS(60);
	while (!window.ShouldClose()) {
		cameraControl(cam);
		window.BeginDrawing();
		window.ClearBackground();
		cam.BeginMode();
		physicsServer.rebuild();
		for (Particle *p : particles) {
			colliders.clear();
			physicsServer.getCollisions(*p, colliders);
			for (CollisionObject2D *pn2 : colliders) {
				if (*p != *pn2) {
					Particle *p2 = dynamic_cast<Particle *>(pn2);
					p->collideWith(*p2, 0.5f);
				}
			}
			for (Node2D *p2 : particles) {
				if (*p != *p2) {
					p->applyNewtonianGravity(p2->getPos());
				}
			}
			p->updatePhysics();
		}
		renderServer.render(window, cam);
		cam.EndMode();
		window.DrawFPS();
		window.EndDrawing();
	}
	for (Node2D *p : particles) {
		delete p;
	}
	PhysicsServer::deleteInstance();
	return (0);
}
