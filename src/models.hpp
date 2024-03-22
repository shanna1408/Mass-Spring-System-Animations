#pragma once

#include <vector>
#include <givr.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp> // lerp

namespace simulation {
	namespace primatives {
		//Mass points used in all simulations
		struct Mass {
			bool fixed = false;
			glm::vec3 p = glm::vec3(0.f);
			glm::vec3 v = glm::vec3(0.f);
			glm::vec3 a = glm::vec3(0.f);
			glm::vec3 p_last = glm::vec3(0.f);
			glm::vec3 p_last_last = glm::vec3(0.f);
			// Spring force
			glm::vec3 f_s = glm::vec3(0.f);
			// Damping force
			glm::vec3 f_d = glm::vec3(0.f);
			// Total force
			glm::vec3 f_b = glm::vec3(0.f);
			// Gravity
			glm::vec3 f_g = glm::vec3(0.f);
			float mass=1;
			void calc_a(glm::vec3 f_b){
				a = f_b/mass;
			}
			void verlet_calc_p(float dt){
				p.y = p_last.y + (p_last.y - p_last_last.y) + (a.y*(dt*dt));
			}
			void verlet_calc_v(float dt){
				v.y = (p.y-p_last.y)/dt;
				p_last_last = p_last;
				p_last = p;
			}
			// ***** Must update v first *****
			void semi_implicit_p(float dt){
				p.y = p.y + v.y * dt;
			}
			void semi_implicit_v(float dt){
				v.y = v.y + a.y * dt;
			}
		};

		//Spring connections used in all simulations
		struct Spring {
			Mass* mass_a = nullptr;
			Mass* mass_b = nullptr;
			glm::vec3 s = glm::vec3(0.f);
			// Spring force
			glm::vec3 f_s = glm::vec3(0.f);
			// Damping force
			glm::vec3 f_d = glm::vec3(0.f);
			// Spring constant
			float k = 20;
			//Spring resting length
			float r = 0;
			// Spring current length
			float l = 0;
			// Damping coefficient
			float c;
			float critical_damp(float mass){
				return (2.f*sqrt(k*mass));
			}
		};

		//Face connections used (can just be a render primative or a simulation primatives for the bonus)
		struct Face {
			Mass* mass_a = nullptr;
			Mass* mass_b = nullptr;
			Mass* mass_c = nullptr;
		};
	} // namespace primatives

	namespace models {
		//If you want to use a different view, change this and the one in main
		using ModelViewContext = givr::camera::ViewContext<givr::camera::TurnTableCamera, givr::camera::PerspectiveProjection>;
		// Abstract class used by all models
		class GenericModel {
		public:
			virtual void reset() = 0;
			virtual void step(float dt) = 0;
			virtual void render(const ModelViewContext& view) = 0;
		};

		//Model constructing a single spring
		class MassOnSpringModel : public GenericModel {
		public:
			MassOnSpringModel();
			void reset();
			void step(float dt);
			void render(const ModelViewContext& view);

			//Simulation Constants (you can re-assign values here from imgui)
			glm::vec3 g = { 0.f, -9.81f, 0.f };

		private:
			//Simulation Parts
			primatives::Mass mass_a;
			primatives::Mass mass_b;
			primatives::Spring spring;
			bool released = false;

			//Render
			givr::geometry::Sphere mass_geometry; 
			givr::style::Phong mass_style;
			givr::InstancedRenderContext<givr::geometry::Sphere, givr::style::Phong> mass_render;

			givr::geometry::MultiLine spring_geometry;
			givr::style::LineStyle spring_style;
			givr::RenderContext<givr::geometry::MultiLine, givr::style::LineStyle> spring_render;
		};

		//Model constructing a chain of springs
		class ChainPendulumModel : public GenericModel {
		public:
			ChainPendulumModel();
			void reset();
			void step(float dt);
			void render(const ModelViewContext& view);

			//Simulation Constants (you can re-assign values here from imgui)
			glm::vec3 g = { 0.f, -9.81f, 0.f };

		private:

			//Simulation Parts
			std::vector<primatives::Mass> masses;
			std::vector<primatives::Spring> springs;

			//Render
			givr::geometry::Sphere mass_geometry;
			givr::style::Phong mass_style;
			givr::InstancedRenderContext<givr::geometry::Sphere, givr::style::Phong> mass_render;

			givr::geometry::MultiLine spring_geometry;
			givr::style::LineStyle spring_style;
			givr::RenderContext<givr::geometry::MultiLine, givr::style::LineStyle> spring_render;
		};

		// TO-DO: Fully implements the last two using the scheme provided above
		//class CubeOfJellyModel : public GenericModel {...}; //should be at least 4 in each direction
		//class HangingClothModel : public GenericModel {...}; //should be at least 8 in each direction

	} // namespace models
} // namespace simulation