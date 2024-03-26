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
			bool in_collision = false;
			glm::vec3 p = glm::vec3(0.f);
			glm::vec3 v = glm::vec3(0.f);
			glm::vec3 a = glm::vec3(0.f);
			// Total force
			glm::vec3 f_i = glm::vec3(0.f);
			// Gravity
			glm::vec3 f_g = glm::vec3(0.f);
			float mass;
			void calc_a(){
				a = f_i/mass;
			}
			// ***** Must update v first *****
			void semi_implicit_p(float dt){
				p = p + v * dt;
			}
			void semi_implicit_v(float dt){
				v = v + a * dt;
			}
			void integrate(float dt){
				calc_a();
				semi_implicit_v(dt);
				semi_implicit_p(dt);
			}
			void calc_collision(float ground){
				if (p.y<ground){
					in_collision = true;
				} else {
					in_collision = false;
				}
				if (in_collision){
					float k = 50000;
					glm::vec3 plane_normal = {0, 1, 0};
					float d = glm::dot(plane_normal, p-glm::vec3({p.x, ground, p.z}));
					glm::vec3 f_coll = -k*d*plane_normal;
					f_i += f_coll;
				} 
			}
		};

		//Spring connections used in all simulations
		struct Spring {
			Mass* mass_a = nullptr;
			Mass* mass_b = nullptr;
			glm::vec3 s = glm::vec3(0.f);
			// s normal
			glm::vec3 s_n = glm::vec3(0.f);
			// Spring force
			glm::vec3 f_s = glm::vec3(0.f);
			// Damping force
			glm::vec3 f_d = glm::vec3(0.f);
			// Spring constant
			float k;
			// //Spring resting length
			float r;
			// // Spring current length
			float l;
			// Damping coefficient
			float c;
			float critical_damp(float mass){
				return (2.f*sqrt(k*mass));
			}
			void calc_spring_size(){
				s = mass_a->p - mass_b->p;
				l = glm::length(s);
				s_n = glm::normalize(s);
			}
			void calc_fs(){
				f_s = -k*(l-r)*(s_n);
			}
			void calc_fd(){
				f_d = -c*(glm::dot((mass_a->v-mass_b->v),s_n))*(s_n);
			}
			// apply force to masses
			void apply_forces(){
				calc_spring_size();
				calc_fs();
				calc_fd();
				mass_a->f_i += f_s + f_d;
				mass_b->f_i += - f_s - f_d;
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
			float mass_size = 0.5f;
			float k = 100.f;
			
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

		class CubeOfJellyModel : public GenericModel {
			public:
				CubeOfJellyModel();
				void reset();
				void step(float dt);
				void render(const ModelViewContext& view);

				//Simulation Constants (you can re-assign values here from imgui)
				glm::vec3 g = { 0.f, -9.81f, 0.f };
				float width = 7;
				float height = 4;
				float length = 4;

			private:
				//Simulation Parts
				std::vector<std::vector<std::vector<primatives::Mass*>>> cube;
				std::vector<primatives::Mass> masses;
				std::vector<primatives::Spring> springs;
				float ground = -20;
				float r = 1;
				float k = 2000;

				//Render
				givr::geometry::TriangleSoup jelly_geometry;
				givr::style::Phong jelly_style;
				givr::RenderContext<givr::geometry::TriangleSoup, givr::style::Phong> jelly_render;

				givr::geometry::TriangleSoup floor_geometry;
				givr::style::Phong floor_style;
				givr::RenderContext<givr::geometry::TriangleSoup, givr::style::Phong> floor_render;
		}; //should be at least 4 in each direction
		class HangingClothModel : public GenericModel {
			public:
				HangingClothModel();
				void reset();
				void step(float dt);
				void render(const ModelViewContext& view);

				//Simulation Constants (you can re-assign values here from imgui)
				glm::vec3 g = { 0.f, -9.81f, 0.f };
				float width = 15;
				float height = 8;

			private:
				//Simulation Parts
				std::vector<std::vector<primatives::Mass*>> cloth;
				std::vector<primatives::Mass> masses;
				std::vector<primatives::Spring> springs;
				float r = 1;
				float k = 100;

				//Render
				givr::geometry::Sphere mass_geometry; 
				givr::style::Phong mass_style;
				givr::InstancedRenderContext<givr::geometry::Sphere, givr::style::Phong> mass_render;

				givr::geometry::MultiLine spring_geometry;
				givr::style::LineStyle spring_style;
				givr::RenderContext<givr::geometry::MultiLine, givr::style::LineStyle> spring_render;

				givr::geometry::TriangleSoup cloth_geometry;
				givr::style::Phong cloth_style;
				givr::RenderContext<givr::geometry::TriangleSoup, givr::style::Phong> cloth_render;
		}; //should be at least 8 in each direction

	} // namespace models
} // namespace simulation