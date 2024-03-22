#include "models.hpp"
#include <iostream>
#include <math.h>

namespace simulation {
	namespace primatives {
		//No functions for structs, yet...
	}// namespace primatives

	namespace models {
		//////////////////////////////////////////////////
		////            MassOnSpringModel             ////----------------------------------------------------------
		//////////////////////////////////////////////////

		MassOnSpringModel::MassOnSpringModel()
			: mass_geometry(givr::geometry::Radius(0.2f))
			, mass_style(givr::style::Colour(1.f, 0.f, 1.f), givr::style::LightPosition(100.f, 100.f, 100.f))
			, spring_geometry()
			, spring_style(givr::style::Colour(1.f, 0.f, 1.f))
		{
			// Link up (Static elements)
			mass_a.fixed = true;
			mass_b.fixed = false;
			spring.mass_a = &mass_a;
			spring.mass_b = &mass_b;
			// Reset Dynamic elements
			reset();

			// Render
			mass_render = givr::createInstancedRenderable(mass_geometry, mass_style);
			spring_render = givr::createRenderable(spring_geometry, spring_style);
		}
		
		void MassOnSpringModel::reset() {
			//As you add quantities to the primatives, they should be set here.
			mass_a.p = { 0.f,0.f,0.f };
			mass_a.v = { 0.f,0.f,0.f }; // Fixed anyway so doesnt matter if implemented correctly
			mass_b.p = { 0.f,-5,0.f};
			mass_b.v = { 0.f,0.f,0.f };
			spring.r = glm::length(mass_b.p);
			mass_b.mass = 0.5;
			mass_b.f_g.y = -9.81*mass_b.mass;
			// Underdamped: 10% of critical damp
			spring.c = spring.critical_damp(mass_b.mass)*0.1;
			released = false;
			//This model can start vertical and be just a spring in the y direction only (like currently set up)
		}

		void MassOnSpringModel::step(float dt) {
			mass_b.f_s.y = -spring.k*(abs(mass_b.p.y) - spring.r);
			// Damping force
			mass_b.f_d.y = -(spring.c)*mass_b.v.y;
			// Total force on mass b
			mass_b.f_b.y = -mass_b.f_s.y + mass_b.f_d.y + mass_b.f_g.y;
			// Pull the string down
			if (!released && mass_b.p.y>-8){
				mass_b.p.y -= 0.025;
			// Then string go boiiiiingggg
			} else {
				released = true;
				mass_b.calc_a(mass_b.f_b);
				mass_b.semi_implicit_v(dt);
				mass_b.semi_implicit_p(dt);
			}
		}

		void MassOnSpringModel::render(const ModelViewContext& view) {

			//Add Mass render
			givr::addInstance(mass_render, glm::translate(glm::mat4(1.f), mass_a.p));
			givr::addInstance(mass_render, glm::translate(glm::mat4(1.f), mass_b.p));

			//Clear and add springs
			spring_geometry.segments().clear();
			spring_geometry.push_back(
				givr::geometry::Line(
					givr::geometry::Point1(spring.mass_a->p), 
					givr::geometry::Point2(spring.mass_b->p)
				)
			);
			givr::updateRenderable(spring_geometry, spring_style, spring_render);

			//Render
			givr::style::draw(mass_render, view);
			givr::style::draw(spring_render, view);
		}

		//////////////////////////////////////////////////
		////           ChainPendulumModel             ////----------------------------------------------------------
		//////////////////////////////////////////////////

		ChainPendulumModel::ChainPendulumModel() 
			: mass_geometry(givr::geometry::Radius(0.2f))
			, mass_style(givr::style::Colour(1.f, 0.f, 1.f), givr::style::LightPosition(100.f, 100.f, 100.f))
			, spring_geometry()
			, spring_style(givr::style::Colour(1.f, 0.f, 1.f))
		{
			//Link up (Static elements)
			masses.resize(11);
			masses[0].fixed = true;
			for (int i=1; i<11; i++){
				masses[i].fixed = false;
			}

			springs.resize(10);
			for (int i=0; i<10; i++){
				springs[i].mass_a = &masses[i];
				springs[i].mass_b = &masses[i+1];
			}
			//Reset Dynamic elements
			reset();

			// Render
			mass_render = givr::createInstancedRenderable(mass_geometry, mass_style);
			spring_render = givr::createRenderable(spring_geometry, spring_style);
		}

		void ChainPendulumModel::reset() {
			//As you add quantities to the primatives, they should be set here.
			float x = 0;
			for (int i=0; i<11; i++){
				masses[i].p = { x,0.f,0.f };
				masses[i].v = { 0.f,0.f,0.f };
				masses[i].a = { 0.f,0.f,0.f };
				masses[i].f_s = { 0.f,0.f,0.f };
				masses[i].f_d = { 0.f,0.f,0.f };
				masses[i].f_b = { 0.f,0.f,0.f };
				masses[i].mass = 0.5;
				masses[i].f_g.y = -9.81*masses[i].mass;
				x += 2.f;
				// All springs have the same resting length (distance between origin and first mass)
				if (i<10){
					springs[i].s = { 0.f,0.f,0.f };
					springs[i].r = glm::length(masses[1].p);
					springs[i].l = springs[i].r;
					springs[i].c = springs[i].critical_damp(masses[i].mass);
					springs[i].f_s = { 0.f,0.f,0.f };
					springs[i].f_d = { 0.f,0.f,0.f };
				}
			}
			//The model should start non-vertical so we can see swaying action
		}

		void ChainPendulumModel::step(float dt) {
			// TODO: Complete the Chain Pendulum step
			for (int i = 0; i < masses.size(); i++) {
				if (i<masses.size()-1) {
					springs[i].s = masses[i].p - masses[i+1].p;
					springs[i].l = glm::length(springs[i].s);
					glm::vec3 s_n = springs[i].s/springs[i].l;
					springs[i].f_s =  -springs[i].k*(springs[i].l-springs[i].r)*(s_n);
					springs[i].f_d = -springs[i].c*(glm::dot((masses[i].v-masses[i+1].v),s_n))*(s_n);
				}
				if (i==masses.size()-1){
					masses[i].f_s = springs[i-1].f_s;
					masses[i].f_d = springs[i-1].f_d;
				} else if (i>0) {
					masses[i].f_s = - springs[i-1].f_s + springs[i].f_s;
					masses[i].f_d = - springs[i-1].f_d + springs[i].f_d;
					// masses[i+1].f_b = - masses[i].f_s - masses[i].f_d;
				}
				// masses[i].f_b = masses[i].f_s + masses[i].f_d + masses[i].f_g;
				masses[i].f_b = masses[i].f_s  + masses[i].f_g;
				std::cout << "i: " << i << std::endl;
				std::cout << " f_s: [" << masses[i].f_s.x << "," << masses[i].f_s.y << "," << masses[i].f_s.z << "]" << std::endl;
				std::cout << " f_d: [" << masses[i].f_d.x << "," << masses[i].f_d.y << "," << masses[i].f_d.z << "]" << std::endl;
				std::cout << " f_b: [" << masses[i].f_b.x << "," << masses[i].f_b.y << "," << masses[i].f_b.z << "]" << std::endl;
			}
			for (int i = 1; i < masses.size(); i++) {
				masses[i].calc_a(masses[i].f_b);
				masses[i].semi_implicit_v(dt);
				masses[i].semi_implicit_p(dt);
			}
		}

		void ChainPendulumModel::render(const ModelViewContext& view) {

			//Add Mass render
			for (const primatives::Mass& mass : masses) {
				givr::addInstance(mass_render, glm::translate(glm::mat4(1.f), mass.p));
			}

			//Clear and add springs
			spring_geometry.segments().clear();
			for (const primatives::Spring& spring : springs) {
				spring_geometry.push_back(
					givr::geometry::Line(
						givr::geometry::Point1(spring.mass_a->p),
						givr::geometry::Point2(spring.mass_b->p)
					)
				);
			}
			givr::updateRenderable(spring_geometry, spring_style, spring_render);

			//Render
			givr::style::draw(mass_render, view);
			givr::style::draw(spring_render, view);
		}
	} // namespace models
} // namespace simulation