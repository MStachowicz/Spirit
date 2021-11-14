class Context;

class Renderer
{
public:
	bool initialise();
	void drawLoop();
	
	private:
		Context *mGraphicsContext;
};