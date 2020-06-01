#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <iostream>

#include <shader.hpp>
#include <rc_spline.h>
// include a math.h library
#include <math.h>

struct Orientation {
	// Front
	glm::vec3 Front;
	// Up
	glm::vec3 Up;
	// Right
	glm::vec3 Right;
	// origin
	glm::vec3 origin;
};


class Track
{
public:

	// VAO
	unsigned int VAO;

	// Control Points Loading Class for loading from File
	rc_Spline g_Track;

	// Vector of control points
	std::vector<glm::vec3> controlPoints;
    std::vector<glm::vec3> interpolatedPoints;
	// Track data
	std::vector<Vertex> vertices;
	// indices for EBO
	std::vector<unsigned int> indices;

	// hmax for camera
	float hmax = 0.0f;


	// constructor, just use same VBO as before, 
	Track(const char* trackPath)
	{
		
		// load Track data
		load_track(trackPath);

		create_track();

		setup_track();
	}

	// render the mesh
	void Draw(Shader shader, unsigned int textureID)
    {
        // Draw the objects here.
        // Set the shader properties
        shader.use();
        glm::mat4 track_model;
        shader.setMat4("model", track_model);


        // Set material properties
        shader.setVec3("material.specular", 0.3f, 0.3f, 0.3f);
        shader.setFloat("material.shininess", 16.0f);


        // active proper texture unit before binding
        glActiveTexture(GL_TEXTURE0);
        // and finally bind the textures
        glBindTexture(GL_TEXTURE_2D, textureID);

        // draw mesh
        //std::cout<<"########################"<<vertices.size()<<std::endl;
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());
        glBindVertexArray(0);

        // always good practice to set everything back to defaults once configured.
        glActiveTexture(GL_TEXTURE0);
	}

	// given an s float, find the point
	//  S is defined as the distance on the spline, so s=1.5 is the at the halfway point between the 1st and 2nd control point
	glm::vec3 get_point(float s)
	{
        float i;
        float tau = 0.5f;
        float f = modf(s,&i);
        float index = i;
        // Get the coordinates in the curve
        unsigned long a, b, c, d;
        a = (index-1) >= 0 ?  (unsigned long)(index-1)% controlPoints.size() : (controlPoints.size()-1);
        b = (unsigned long) index % controlPoints.size();
        c = (unsigned long) (index+1) % controlPoints.size();
        d = (unsigned long) (index+2) % controlPoints.size();
       // std::cout<<"a:"<<a<<" b:"<<b<<" c:"<<c<<" d:"<<d<<std::endl;
        return interpolate(controlPoints[a], controlPoints[b], controlPoints[c], controlPoints[d], tau, f);
	}
    //##################################################################
    // Given the distance s, we need to compute the u value in the curve
    //##################################################################
    glm::vec3 estimated_point(float s, float &prev_u, Orientation &prev_p)
    {
        glm::vec3 prev_pos = prev_p.origin;
        glm::vec3 cur_pos;
        float u = prev_u;
        float target_s = s;
        float delta_u = 0.01;
        float delta_dist;
        while (target_s>=0)
        {
            cur_pos = get_point(u);
            glm::vec3 delta_pos = cur_pos - prev_pos;
            delta_dist = sqrt(delta_pos[0]*delta_pos[0] + delta_pos[1]*delta_pos[1] + delta_pos[2]*delta_pos[2]);
            target_s -= delta_dist;
            u+=delta_u;
        }
        prev_u = u-delta_u;
        return cur_pos;

    }
    //##################################################################
    // get the current point's orientation and next_point's orientation
     //##################################################################
    void get_Orientation(bool init, glm::vec3 origin_1, glm::vec3 origin_2,Orientation &cur_points, Orientation &next_points)
    {
        //set the origin for cur_points and next_points.
        cur_points.origin = origin_1;
        next_points.origin = origin_2;

        // if i == 0, set the initial cur_points.
        if(init)
        {
            cur_points.Front = glm::normalize(cur_points.origin - interpolatedPoints[interpolatedPoints.size()-1]);
            cur_points.Up = glm::vec3(0, 1, 0);     //use the y axis as the up direction in the world coordinate.
            cur_points.Right = glm::cross(cur_points.Front, cur_points.Up);
        }

        next_points.Front = glm::normalize(next_points.origin - cur_points.origin);
        next_points.Right = glm::normalize(glm::cross(next_points.Front, cur_points.Up));
        next_points.Up = glm::cross(next_points.Right, next_points.Front);
    }

	void delete_buffers()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}

private:
	

	/*  Render data  */
	unsigned int VBO, EBO;

	void load_track(const char* trackPath)
	{
		// Set folder path for our projects (easier than repeatedly defining it)
		g_Track.folder = "../Project_2/Media/";

		// Load the control points
		g_Track.loadSplineFrom(trackPath);

	}

	// Implement the Catmull-Rom Spline here
	//     Given 4 points, a tau and the u value 
	//     Since you can just use linear algebra from glm, just make the vectors and matrices and multiply them.  
	//     This should not be a very complicated function
	glm::vec3 interpolate(glm::vec3 pointA, glm::vec3 pointB, glm::vec3 pointC, glm::vec3 pointD, float tau, float u)
	{
        glm::mat4 mu = glm::mat4(0.0f, 1.0f, 0.0f, 0.0f,  // first column
                                 -tau, 0.0f, tau, 0.0f, // second column
                                 2*tau, tau-3, 3-2*tau, -tau, // third column
                                 -tau, 2-tau, tau-2, tau); // fourth column
        glm::mat4x3 points = glm::mat4x3(pointA,pointB,pointC,pointD);
        glm::vec4 coefficient = glm::vec4(1, u, u*u, u*u*u);
        glm::vec3 fu = points*mu*coefficient;
		// Just returning the first point at the moment, you need to return the interpolated point.  
        return fu;
	}

	// Here is the class where you will make the vertices or positions of the necessary objects of the track (calling subfunctions)
	//  For example, to make a basic roller coster:
	//    First, make the vertices for each rail here (and indices for the EBO if you do it that way).  
	//        You need the XYZ world coordinates, the Normal Coordinates, and the texture coordinates.
	//        The normal coordinates are necessary for the lighting to work.  
	//    Second, make vector of transformations for the planks across the rails
	void create_track()
	{
        // ####################################################################################################
		// Create the vertices and indices (optional) for the rails
		//    One trick in creating these is to move along the spline and 
		//    shift left and right (from the forward direction of the spline) 
		//     to find the 3D coordinates of the rails.

		// Create the plank transformations or just creating the planks vertices
		//   Remember, you have to make planks be on the rails and in the same rotational direction 
		//       (look at the pictures from the project description to give you ideas).  
        // ####################################################################################################
        float s = 0.0f;  //set the minimum unit for interpolating the curve.
        float delta_s = 0.05f;
        // ####################################################################################################
		// Here is just visualizing of using the control points to set the box transformatins with boxes. 
		//       You can take this code out for your rollercoster, this is just showing you how to access the control points
        // ####################################################################################################
		glm::vec3 currentpos = glm::vec3(-2.0f, 0.0f, -2.0f);
		/* iterate throught  the points	g_Track.points() returns the vector containing all the control points */
		for (pointVectorIter ptsiter = g_Track.points().begin(); ptsiter != g_Track.points().end(); ptsiter++)
		{
			/* get the next point from the iterator */
			glm::vec3 pt(*ptsiter);

			// Print the Box
            //std::cout << pt.x << "  " << pt.y << "  " << pt.z << std::endl;

			/* now just the uninteresting code that is no use at all for this project */
			currentpos += pt;
			//  Mutliplying by two and translating (in initialization) just to move the boxes further apart.  
            controlPoints.push_back(currentpos*2.0f);
            std::cout<< currentpos.x << " " <<currentpos.y<<" "<<currentpos.z<<std::endl;
            //std::cout<<"##########################"<<std::endl;
		}
        // ####################################################################################################
        // Compute the interpolated point.
        // ####################################################################################################
        while(s<= controlPoints.size())
        {
            glm::vec3 interpolated_point = get_point(s);
            s += delta_s;

            //store the interpolatedPoints in the vector.
            interpolatedPoints.push_back(interpolated_point);
        }
        // ####################################################################################################
        Orientation cur_points,next_points;
        glm::vec3 offset0 = glm::vec3(0,0.1,0.5);
        glm::vec3 offset1 = glm::vec3(0.5, 0.3, 0.1);
        glm::vec3 offset2 = glm::vec3(-0.5, 0.4, 0.1);
        glm::vec3 offset3 = glm::vec3(-0.5, 0.4, 0.1);
        for(unsigned long i = 0; i<interpolatedPoints.size(); i++)
        {
            // if i == 0, set the initial cur_points.
            if(i==0)
            {
                get_Orientation(true, interpolatedPoints[i], interpolatedPoints[(i+1)%interpolatedPoints.size()], cur_points, next_points);
            }
            else
            {
                get_Orientation(false, interpolatedPoints[i], interpolatedPoints[(i+1)%interpolatedPoints.size()], cur_points, next_points);
            }

            //make the rail part
            makeRailPart(cur_points, next_points, offset0);
            makeRailPart(cur_points, next_points, offset1);
            makeRailPart(cur_points, next_points, offset2);

            if( i% (unsigned long)(1/delta_s) == 0 )
            {
                 makeRailStandPart(next_points, offset0);
            }

            //  set the cur_points as the next_points
            cur_points = next_points;
        }

	}

	// Given 3 Points, create a triangle and push it into vertices (and EBO if you are using one)
		// Optional boolean to flip the normal if you need to
    void make_triangle(glm::vec3 pointA, glm::vec3 pointB, glm::vec3 pointC, bool flipNormal)
    {
        Vertex v1, v2, v3;
        v1.Position = pointA;
        v2.Position = pointB;
        v3.Position = pointC;

        if(flipNormal)
        {
            v1.TexCoords.x = 0.0f;
            v1.TexCoords.y = 1.0f;
            v2.TexCoords.x = 0.0f;
            v2.TexCoords.y = 0.0f;
            v3.TexCoords.x = 1.0f;
            v3.TexCoords.y = 1.0f;

            //set the normal vector
            set_normals(v1, v2, v3, flipNormal);
        }
        else
        {
            v1.TexCoords.x = 1.0f;
            v1.TexCoords.y = 1.0f;
            v2.TexCoords.x = 1.0f;
            v2.TexCoords.y = 0.0f;
            v3.TexCoords.x = 0.0f;
            v3.TexCoords.y = 0.0f;

            //set the normal vector
            set_normals(v1, v2, v3, flipNormal);
        }

        vertices.push_back(v1);
        vertices.push_back(v2);
        vertices.push_back(v3);
	}

    // Draw the rail stand cube.
    void makeRailStandPart(Orientation ori_cur, glm::vec3 offset)
    {
        glm::vec3 pointI = glm::vec3(ori_cur.origin - glm::vec3(0.0f, 1.0f, 0.0f)*offset[1] - ori_cur.Right*0.2f + ori_cur.Front*0.2f);
        glm::vec3 pointJ = glm::vec3(ori_cur.origin - glm::vec3(0.0f, 1.0f, 0.0f)*offset[1] + ori_cur.Right*0.2f + ori_cur.Front*0.1f);
        glm::vec3 pointK = glm::vec3(ori_cur.origin - glm::vec3(0.0f, 1.0f, 0.0f)*offset[1] + ori_cur.Right*0.1f - ori_cur.Front*0.1f);
        glm::vec3 pointQ = glm::vec3(ori_cur.origin - glm::vec3(0.0f, 1.0f, 0.0f)*offset[1] - ori_cur.Right*0.1f - ori_cur.Front*0.1f);

        glm::vec3 pointM = glm::vec3(ori_cur.origin - glm::vec3(0.0f, 1.0f, 0.0f)*30.0f - ori_cur.Right*0.2f + ori_cur.Front*0.2f);
        glm::vec3 pointN = glm::vec3(ori_cur.origin - glm::vec3(0.0f, 1.0f, 0.0f)*30.0f + ori_cur.Right*0.2f + ori_cur.Front*0.2f);
        glm::vec3 pointV = glm::vec3(ori_cur.origin - glm::vec3(0.0f, 1.0f, 0.0f)*30.0f + ori_cur.Right*0.2f - ori_cur.Front*0.2f);
        glm::vec3 pointU = glm::vec3(ori_cur.origin - glm::vec3(0.0f, 1.0f, 0.0f)*30.0f - ori_cur.Right*0.2f - ori_cur.Front*0.2f);

        make_triangle(pointI, pointJ, pointM, true);
        make_triangle(pointM, pointN, pointJ, false);

        make_triangle(pointJ, pointK, pointN, true);
        make_triangle(pointN, pointV, pointK, false);

        make_triangle(pointK, pointQ, pointV, true);
        make_triangle(pointV, pointU, pointQ, false);

        make_triangle(pointQ, pointI, pointU, true);
        make_triangle(pointU, pointM, pointI, false);
    }

	// Given two orintations, create the rail between them.  Offset can be useful if you want to call this for more than for multiple rails
    void makeRailPart(Orientation ori_prev, Orientation ori_cur, glm::vec3 offset)
	{
       // bool flipNormal = false;

        glm::vec3 pointA = glm::vec3(ori_prev.origin + ori_prev.Up*offset[1] - ori_prev.Right*offset[2]) + ori_prev.Right*offset[0];
        glm::vec3 pointB = glm::vec3(ori_prev.origin + ori_prev.Up*offset[1] + ori_prev.Right*offset[2]) + ori_prev.Right*offset[0];
        glm::vec3 pointC = glm::vec3(ori_prev.origin - ori_prev.Up*offset[1] + ori_prev.Right*offset[2]) + ori_prev.Right*offset[0];
        glm::vec3 pointD = glm::vec3(ori_prev.origin - ori_prev.Up*offset[1] - ori_prev.Right*offset[2]) + ori_prev.Right*offset[0];

        glm::vec3 pointE = glm::vec3(ori_cur.origin + ori_cur.Up*offset[1] - ori_cur.Right*offset[2]) + ori_cur.Right*offset[0];
        glm::vec3 pointF = glm::vec3(ori_cur.origin + ori_cur.Up*offset[1] + ori_cur.Right*offset[2]) + ori_cur.Right*offset[0];
        glm::vec3 pointG = glm::vec3(ori_cur.origin - ori_cur.Up*offset[1] + ori_cur.Right*offset[2]) + ori_cur.Right*offset[0];
        glm::vec3 pointH = glm::vec3(ori_cur.origin - ori_cur.Up*offset[1] - ori_cur.Right*offset[2]) + ori_cur.Right*offset[0];

        make_triangle(pointA, pointB, pointE, true);
        make_triangle(pointE, pointF, pointB, false);

        make_triangle(pointD, pointC, pointH, true);
        make_triangle(pointH, pointG, pointC, false);

        make_triangle(pointA, pointD, pointE, true);
        make_triangle(pointE, pointH, pointD, false);

        make_triangle(pointB, pointC, pointF, true);
        make_triangle(pointF, pointG, pointC, false);
	}

	// Find the normal for each triangle uisng the cross product and then add it to all three vertices of the triangle.  
	//   The normalization of all the triangles happens in the shader which averages all norms of adjacent triangles.   
	//   Order of the triangles matters here since you want to normal facing out of the object.  
    void set_normals(Vertex &p1, Vertex &p2, Vertex &p3, bool flipNormal)
	{
        glm::vec3 normal;
        if(flipNormal)
        {
            normal = glm::cross(p2.Position - p1.Position, p3.Position - p1.Position);
        }
        else
        {
            normal = -glm::cross(p2.Position - p1.Position, p3.Position - p1.Position);
        }
		p1.Normal += normal;
		p2.Normal += normal;
		p3.Normal += normal;
	}

	void setup_track()
	{
	   // Like the heightmap project, this will create the buffers and send the information to OpenGL

        // create buffers/arrays
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        //glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/3/2 array which
        // again translates to 3/3/2 floats which translates to a byte array.
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

       // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
      //  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // set the vertex attribute pointers
        // vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // vertex normal coords
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        glBindVertexArray(0);

	
	
	}

};
