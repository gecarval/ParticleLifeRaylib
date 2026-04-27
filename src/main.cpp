#include "../classes/HashCollision.hpp"
#include "../classes/Particle.hpp"
#include "../classes/ui/Button.hpp"

int main(void) {
	std::vector<Particle *> particles;
	particles.reserve(1000);
	for (int i = 0; i < 1000; ++i) {
		raylib::Vector2 pos(rand() % 800, rand() % 600);
		Particle	   *newParticle = new Particle(pos);
		particles.push_back(newParticle);
	}
	raylib::Window window(800, 600);
	window.SetTargetFPS(60);
	while (!window.ShouldClose()) {
		window.Drawing();
		window.ClearBackground();
		window.DrawFPS();
		for (Particle *p : particles) {
			Particle &particle = *p;
			particle.update();
			particle.applyGravity();
			particle.draw();
			particle.debugDraw();
		}
		window.EndDrawing();
	}
	for (Particle *p : particles) {
		delete p;
	}
	return (0);
}
