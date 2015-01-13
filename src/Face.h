/******************************************************************************
 *
 * *.obj format geometric face data type
 *
 * @file Face.h
 * @author Michael Woods
 *
 ******************************************************************************/

#include <cstring>

#ifndef FACE_H
#define FACE_H

#include <iostream>

/******************************************************************************/

class Face 
{
	public:
		int id;   // Face identifer
		int v[3]; // Vertex indices
		int t[3]; // Vertex-texture indices
		int n[3]; // Vertex-normal indices

	public:
		Face()
		{
			this->id   = -1;
			this->v[0] = this->v[1] = this->v[2] = -1;
			this->t[0] = this->t[1] = this->t[2] = -1;
			this->n[0] = this->n[1] = this->n[2] = -1;
		}

		Face(int _id, int v[3]) :
			id(_id)
		{
			std::memcpy(&this->v[0], &v[0], sizeof(this->v[0]) * 3);
			std::memset(&this->t[0], -1, sizeof(this->t[0]) * 3);
			std::memset(&this->n[0], -1, sizeof(this->n[0]) * 3);
		}

		Face(int _id, int v[3], int t[3]) :
			id(_id)
		{
			std::memcpy(&this->v[0], &v[0], sizeof(this->v[0]) * 3);
			std::memcpy(&this->t[0], &t[0], sizeof(this->t[0]) * 3);
			std::memset(&this->n[0], -1, sizeof(this->n[0]) * 3);
		}

		Face(int _id, int v[3], int t[3], int n[3]) :
			id(_id)
		{
			std::memcpy(&this->v[0], &v[0], sizeof(this->v[0]) * 3);
			std::memcpy(&this->t[0], &t[0], sizeof(this->t[0]) * 3);
			std::memcpy(&this->n[0], &n[0], sizeof(this->n[0]) * 3);
		}

		Face(const Face& other) :
			id(other.id)
		{
			std::memcpy(&this->v[0], &other.v[0], sizeof(this->v[0]) * 3);
			std::memcpy(&this->t[0], &other.t[0], sizeof(this->t[0]) * 3);
			std::memcpy(&this->n[0], &other.n[0], sizeof(this->n[0]) * 3);
		}

		friend std::ostream& operator<<(std::ostream& s, const Face& face);
};

/******************************************************************************/

#endif
