# CPSC 687 Assignment 2
# Shanna Hollingworth Winter 2024

## Instructions:

* To run this program, click the a2_base.exe inside of the build/Debug folder (Compiled for Windows 11 using Visual Studio C++ compiler).

* Use the `RGB` selectors to change the background colour of the animation.

* `Reset View` will return the camera to it's starting position.

* The `Iterations per Frame` silder allows you to control the speed at which the animation plays. The default setting is always 1 which is also the lowest setting, and the highest option is 100.

* Use the dropdown menu labeled `Model` to select the simulation to test

* To start the animation, check `Play Simulation`. To pause the simulation, uncheck this.

* `Reset Simulation` will move the objects back to their starting position.

* A button labelled `Step Simulation` will appear when `Play Simulation` is not checked. Click this button to move through the simulation slowly, one iteration at a time.

* The `Simulation dt` slider allows you to adjust the time jump used in the simulation. This is set to a different default value per simulation, and increasing it too much may cause things to break.

## Simulation 1 (Mass on Spring)

For simulation 1, the only necessary components were two masses, one being fixed and the other unfixed, and a spring. First, we define a mass `m`, which I chose to be 0.5, and then a rest length `r` for the spring equal to it's starting position to define the length of the spring when it is not stretched, which for this simulation was 5. I also initialized the force of gravity `F_g`for the mass at this stage, as it will never be changed; $F_g = g*m$, where $g=-9.81m^2$. This is derived from the acceleration equation, $a=F/m$, since g represents the acceleration of gravity. We then initialize all of the starting acceleration, velocity, and force vectors to 0, and the position vectors to the respective mass starting positions.

Next, we calculate the forces acting on the masses at each time step. For this simulation, this comprises of the spring force, the dampening force, and the force of gravity. For each spring, we first, we calculate the current spring vector `s`, then get the length of the vector `l`, and finally normalize `s` to get `s_n`. We require these values in order to calculate the spring and damping forces.

To calculate the spring force, we use Hooke's Law, $f_s = -k*(l-r)*(s_n)$. Hooke's Law essentially describes how a force applied to a string will cause it to stretch or deform. It states that stretching or deforming a spring creates a force proportional to the distance between the current length and rest length. The spring constant k describes how flexible or stiff the spring is. The equation is negative because the force is meant to counteract the other forces pulling the length of the spring above or below it's rest length(Which is calculated by $l-r$), in an effort to bring it back to the equilibrium length `r`. For this model, I set `k` to 15, as it created some stiffness while still allowing for a good bounce.

The dampening force `f_d` describes the force which slows the oscillations and eventually bring them to a stop. Without the dampening force, the oscillations would continue forever, and the higher the dampening force the fewer oscillations which occur before the length of the spring returns to and stays at it's rest length. We can calculate the dampening force using the critical dampening force as a base; this is calculated by $2*sqrt(k*mass)$. The critical dampening force is the value of `f_d` that brings the spring back to rest as quickly as possible. Any higher value results in the spring being overdamped, where it returns directly to the rest length but rather slowly, and any lower of a value results in the spring being underdamped, where it oscillates a few times before coming to a rest. For this model, as only one spring was presented, I chose to extremely underdamp the model to achieve the desired simulation, setting `f_d` at 10% of the critical dampening force.

Finally, I calculated the total force acting on the mass by combining the forces. This step requires Newton's third law, which states that every force is accompanied by an equal an opposite force. In practice, this means that the force acting on the first mass, in this case the fixed mass, is $F_i=F_s+F_d+F_g$, and the force acting on the second mass is equal, but in the opposite direction(not including gravity of course)- $F_i=-F_s-F_d+F_g$. If there were more masses and springs chained together, this would propogate downwards, but we'll talk about that in simulation 2!

Now that F_i has been calculated, we are able to do our integration step. First, we calculate the acceleration as $a=F_i/m$. To calculate the velocity and position, I used the semi-implicit euler method, which first updates the velocity using the current velocity and acceleration with regards to the time step, and then updates the position using this new velocity alongside the current position, again with regards to the time step. This method is more stable than the euler method because it takes into account the current and next time steps. The equations are as follows: $v_{new} = v_{curr} + a * dt$, and $p_{new} = p_{curr} + v_{new} * dt$. Et voila, we have now defined the next position of our mass, and we have successfully broken down all the math required to make a spring go boing!:)

## Simulation 2 (Chain Pendulum)

The second simulation is comprised of 11 masses and 10 springs, with the first mass once again being fixed. For this simulation, I once again selected mass `m` to be 0.5, but this time I set the rest length `r` as only 1.5, as otherwise the chain is too long. I once again initialized the force of gravity `F_g` for each of the masses to be $F_g = g*m$, where $g=-9.81m^2$, all of the starting acceleration, velocity, and force vectors to 0, and the position vectors to the respective mass starting positions.

Next, we calculate the total forces acting on each mass, by first calculating `f_s` and `f_d`. These calculations are exactly the same as described in simulation 1. For this model, I upped my spring constant `k` significantly to 100, as I wanted the spring to mostly hold it length but stretch a little bit. I also chose to increase the dampening constant a bit to limit the oscillations, to 25% of the critical dampening constant. Now, once again, for each spring, the force of the spring acting on the first mass $F_i=F_s+F_d+F_g$, and by Newton's law, the force of the spring on the second mass is equal but in the opposite direction, $F_i=-F_s-F_d+F_g$. However, as the masses are all chained together by springs, most masses are connected to two springs. So each mass will receive a 'negative' force from the spring before it, which will partly be counteracted by a 'positive' force from the spring after it. I put 'positive' and 'negative' in quotations because that doesnt mean these forces themselves are positive or negative, it wholly depends on the calculations. For this simulation, we also account for a new force, viscous dampening `F_{air}, which is just the resistant force pushing back against each mass as it moves through a substance. The equation is equal to $-k*v$, where k is a constant that defines how strong the push back force is. Because the substance is just air which is not very thick at all, we set a low k, 0.05. Now $F_i=F_i+F_{air}$.

In the integration step, the acceleration, position, and velocity are calculated in the exact same way. It's worth noting that the `F_i` used to calculate acceleration should be the sum of all forces acting on the mass, but we account for this already so nothing more needs to be done here.

## Simulation 3 (Jelly Cube)

Simulation 3 is comprised of masses and springs which form a 3-dimensional cube like shape, with 7 masses in the x direction as the `width`, and 4 masses in the y and z directions as the `height` and `length`. For this simulation, I set mass `m` to be 0.1, and each mass to be a length of 1 apart. As in the other two simulations, I initialized the force of gravity `F_g`, all of the starting acceleration, velocity, and force vectors to 0, and the position vectors to the respective mass starting positions. In order to start the cube out being rotated, I multiplied the calculated starting positions for each mass by rotation vectors defining first a 45 degree rotation around z, and then a 45 degree rotation around x. The rotations defined by the rotation matrices multiplied by the current position are as follows:

z_rotation= [x*cos(45)-y*sin(45), x*sin(45)+y*cos(45), z]

x_rotation= [x, y*cos(45)-z*sin(45), y*sin(45)-z*cos(45)]

Next, I calculated the various springs connecting the cube. To prevent stretching, I defined springs between each mass and it's direct neighbours on the x, y, and z axes. Then, to prevent shearing, I placed springs between each mass and each of it's diagonal neighbours on every plane. Finally, to help maintain the structural integrity of the cube, I placed springs between each mass and it's absolute diagonal opposite within each cube the mass is part of. In order to achieve this, I looped through each mass, and for each mass, I looped through all the masses already placed. If the distance between the masses was less than or equal to the threshold defined by the longest needed spring(The last one I described), I added a spring. The rest length for each spring was set to this calculated distance.

Next, we calculate the total forces acting on each mass, by first calculating `f_s`, `f_d`, and `f_air`. These calculations are exactly the same as described in simulation 1. For this model, I upped my spring constant `k` significantly to 2000, to ensure that the cube holds it's shape overall, aside from a bit of jiggle or distortion caused by an immediate force like a corner or edge being momentarily squished into the ground. I kept the dampening constant the same as simulation 2, 25% of the critical dampening constant, as this seemed to achieve the desired effect. For this simulation, we also have to account for a collision force `F_{coll}` as the jelly cube hits the ground and bounces around before coming to a stop. This is calculated using the penalty method. My implementation involves checking whether the y position of each mass is lower than the position of the ground, and if so, setting a boolean to flag the particle as 'in_collision'. I then calculate the collision force as -k*d*plane_normal, where k is some constant, d is the distance from the mass to the plane, and plane_normal is the unit vector normal to the plane. I set k to be 50000, to ensure a strong impact force. Thus, $F_i=Fs+F_d+F_g+F+{air}+F_{coll}$.

The integration step is exactly as described for Simulations 1 and 2. The time step had to be significantly lowered to 0.0002 to ensure that the calculations remained accurate, as there are so many forces and masses and springs involved.

## Simulation 4 (Hanging Cloth)

Simulation 4 is comprised of masses and springs which form a 2-dimensional cloth-like shape, with 15 masses in the x direction as the `width`, and 8 masses in the y direction as the `height`. For this simulation, I set mass `m` to be 0.1, and each mass to be a length of 1 apart. As in the other simulations, I initialized the force of gravity `F_g`, all of the starting acceleration, velocity, and force vectors to 0, and the position vectors to the respective mass starting positions. 

Next, I calculated the various springs connecting the cloth. To prevent stretching, I defined springs between each mass and it's direct neighbour on the x, y axes. Then, to prevent shearing, I placed springs between each mass and each of it's diagonal neighbours. Finally, to prevent out of plane bending, I placed a spring between each mass and the mass exactly 2 away in the vertical and horizontal directions. In order to achieve this, I looped through each mass, and for each mass, I looped through all the masses already placed. If the distance between the masses was less than or equal to the threshold defined by the spring for shearing, or exactly equal to the threshold defined by the spring for stretching, I added a spring. The rest length for each spring was set to this calculated distance.

Next, we calculate the total forces acting on each mass, by first calculating `f_s`, `f_d`, and `f_{air}`. These calculations are exactly the same as described in simulation 1. For this model, I used a spring constant `k` of 100, to ensure the cloth holds it shape but has some room to stretch or bend. I set the dampening constant to 10% of the critical dampening constant, to give lots of room for oscillations. All further force calculations, as well as the integration step, are exactly as described for Simulations 1 and 2.The time step had to be significantly lowered to 0.0002 to ensure that the calculations remained accurate, as there are so many forces and masses and springs involved.

Alas, I didn't get to the wind:(. This cloth is hung in the desert.