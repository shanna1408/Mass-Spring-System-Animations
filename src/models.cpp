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
			mass_b.mass = 0.5;
			mass_b.f_g.y = -9.81*mass_b.mass;
			spring.mass_a = &mass_a;
			spring.mass_b = &mass_b;
			spring.r = 5;
			spring.k = 15;
			// Underdamped: 10% of critical damp
			spring.c = spring.critical_damp(mass_b.mass)*0.1;

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
			released = false;
			//This model can start vertical and be just a spring in the y direction only (like currently set up)
		}

		void MassOnSpringModel::step(float dt) {
			spring.apply_forces();
			// Pull the string down
			if (!released && mass_b.p.y>-8.f){
				mass_b.p.y -= 0.025;
			// Then string go boiiiiingggg
			} else {
				mass_b.f_i += mass_b.f_g;
				released = true;
				mass_b.integrate(dt);
			}
			mass_b.f_i = glm::vec3(0.f);
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
			float mass_size = 0.5f;
			masses.resize(11);
			masses[0].fixed = true;
			for (int i=1; i<11; i++){
				masses[i].fixed = false;
				masses[i].mass = mass_size;
				masses[i].f_g.y = -9.81*masses[i].mass;
			}

			springs.resize(10);
			for (int i=0; i<10; i++){
				springs[i].mass_a = &masses[i];
				springs[i].mass_b = &masses[i+1];
				springs[i].k = 100;
				springs[i].r = 1.5f;
				springs[i].c = springs[i].critical_damp(springs[i].mass_a->mass)*0.25;
			}
			//Reset Dynamic elements
			reset();

			// Render
			mass_render = givr::createInstancedRenderable(mass_geometry, mass_style);
			spring_render = givr::createRenderable(spring_geometry, spring_style);
		}

		void ChainPendulumModel::reset() {
			float x = 0;
			float r = springs[0].r;
			for (primatives::Mass& mass : masses){
				mass.p = { x,0.f,0.f };
				mass.v = { 0.f,0.f,0.f };
				mass.a = { 0.f,0.f,0.f };
				mass.f_s = { 0.f,0.f,0.f };
				mass.f_d = { 0.f,0.f,0.f };
				mass.f_i = { 0.f,0.f,0.f };
				x += r;
			}
			for (primatives::Spring& spring : springs){
				spring.s = { 0.f,0.f,0.f };
				spring.l = r;
				spring.f_s = { 0.f,0.f,0.f };
				spring.f_d = { 0.f,0.f,0.f };
			}
			//The model should start non-vertical so we can see swaying action
		}

		void ChainPendulumModel::step(float dt) {
			for (primatives::Spring& spring : springs){
				spring.apply_forces();
			}
			for (primatives::Mass& mass : masses){
				float k = 0.05;
				glm::vec3 f_air = -k*mass.v;
				mass.f_i += mass.f_g + f_air;
			}
			for (primatives::Mass& mass : masses){
				if (!mass.fixed) {
					mass.integrate(dt);
				}
				mass.f_i = glm::vec3(0.f);
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
		//////////////////////////////////////////////////
		////              CubeOfJelly                 ////----------------------------------------------------------
		//////////////////////////////////////////////////

		CubeOfJellyModel::CubeOfJellyModel() 
			: mass_geometry(givr::geometry::Radius(0.2f))
			, mass_style(givr::style::Colour(1.f, 0.f, 1.f), givr::style::LightPosition(100.f, 100.f, 100.f))
			, spring_geometry()
			, spring_style(givr::style::Colour(1.f, 0.f, 1.f))
			, jelly_geometry()
			, jelly_style(givr::style::Colour(1.f, 0.f, 1.f), givr::style::LightPosition(100.f, 100.f, 100.f))
			, floor_geometry()
			, floor_style(givr::style::Phong(givr::style::Colour(1., 1., 0.1529), givr::style::LightPosition(100.f, 100.f, 100.f)))
		{
			//Link up (Static elements)
			float size = CubeOfJellyModel::length * CubeOfJellyModel::height * CubeOfJellyModel::width;
			masses.resize(size);
			for (int i=0; i<size; i++){
				masses[i].fixed = false;
				masses[i].mass = 0.5;
				masses[i].f_g.y = -9.81*masses[i].mass;
			}

			springs.resize(size*25);
			
			float n = 0;
			float m = 0;
			//Reset to set mass positions, so we can place springs
			reset();
			for (int i=0; i<masses.size(); i++){
				for (int j=0; j<i; j++){
					float d = glm::length(masses[i].p - masses[j].p);
					//Add springs
					if (d<=2.f){
						springs[n].mass_a = &masses[i];
						springs[n].mass_b = &masses[j];
						springs[n].k = 150;
						springs[n].r = d;
						springs[n].c = springs[n].critical_damp(springs[n].mass_a->mass)*0.9f;
						n+=1.f;
					}
				}
			}
			springs.resize(n);

			//Reset Dynamic elements
			reset();

			// Render
			mass_render = givr::createInstancedRenderable(mass_geometry, mass_style);
			spring_render = givr::createRenderable(spring_geometry, spring_style);
			jelly_render = givr::createRenderable(jelly_geometry, jelly_style);

			// float ground = -20;
			floor_geometry.push_back(givr::geometry::Point1(-500.f, ground, 500.f), givr::geometry::Point2(-500.f, ground, -500.f), givr::geometry::Point3(500.f, ground, 500.f));
			floor_geometry.push_back(givr::geometry::Point1(-500.f, ground, -500.f), givr::geometry::Point2(500.f, ground, -500.f), givr::geometry::Point3(500.f, ground, 500.f));
			floor_render = givr::createRenderable(floor_geometry, floor_style);
		}

		void CubeOfJellyModel::reset() {
			//As you add quantities to the primatives, they should be set here.
			float r = 1.0f;
			float n=0;
			for (int i=0; i<CubeOfJellyModel::width; i++){
				for (int j=0; j<CubeOfJellyModel::height; j++){
					for (int k=0; k<CubeOfJellyModel::length; k++){
						masses[n].p = { i*r,j*r,k*r };
						masses[n].v = { 0.f,0.f,0.f };
						masses[n].a = { 0.f,0.f,0.f };
						masses[n].f_s = { 0.f,0.f,0.f };
						masses[n].f_d = { 0.f,0.f,0.f };
						masses[n].f_i = { 0.f,0.f,0.f };
						n++;
					}
				}
			}

			for (primatives::Spring& spring : springs){
				spring.s = { 0.f,0.f,0.f };
				spring.r = r;
				spring.l = r;
				spring.f_s = { 0.f,0.f,0.f };
				spring.f_d = { 0.f,0.f,0.f };
			}
		}

		void CubeOfJellyModel::step(float dt) {
			for (primatives::Spring& spring : springs){
				spring.apply_forces();
			}
			for (primatives::Mass& mass : masses){
				float k = 0.05;
				glm::vec3 f_air = -k*mass.v;
				mass.f_i += mass.f_g + f_air;
				mass.calc_collision(ground);
				mass.integrate(dt);
				mass.f_i = glm::vec3(0.f);
			}
		}

		void CubeOfJellyModel::render(const ModelViewContext& view) {

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

			// auto ts = givr::geometry::TriangleSoup();
			// jelly_geometry.triangles().clear();
			// Loop over all objects in your simulation/animation
			int p1 = 0;
			int p2 = 5;
			int p3 = 15;
			// jelly_geometry.push_back(masses[p1].p, masses[p2].p, masses[p3].p);
			// std::cout << "mass 0: [" << masses[p1].p.x << "," << masses[p1].p.y << "," << masses[p1].p.z << "]" << std::endl;
			// std::cout << "mass 1: [" << masses[p2].p.x << "," << masses[p2].p.y << "," << masses[p2].p.z << "]" << std::endl;
			// std::cout << "mass 2: [" << masses[p3].p.x << "," << masses[p3].p.y << "," << masses[p3].p.z << "]" << std::endl;
			// givr::updateRenderable(jelly_geometry, jelly_style, jelly_render);

			//Render
			givr::style::draw(mass_render, view);
			givr::style::draw(spring_render, view);
			givr::style::draw(jelly_render, view);
			givr::style::draw(floor_render, view);
		};
	} // namespace models
} // namespace simulation