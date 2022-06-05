#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include "mesh.h"

class RenderEngine
{
public:

	void SetMeshes(std::vector<Mesh> meshes)
	{
		this->meshes = meshes;
	}

	void Update()
	{
		for (size_t i = 0; i < meshes.size(); i++)
		{
			meshes[i].Update();
		}
	}

	void Render()
	{
		for (size_t i = 0; i < meshes.size(); i++)
		{
			
			meshes[i].Render();
		}
	}

private:
	std::vector<Mesh> meshes;
};

#endif