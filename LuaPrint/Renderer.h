#pragma once
class Renderer
{
public:
	Renderer(ID3D11Device * device, ID3D11DeviceContext * ctxt);
	void Render();
	~Renderer();
};

