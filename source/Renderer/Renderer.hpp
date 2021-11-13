class Context;

class Renderer
{
public:
	bool initialise();
	void drawLoop();
	Context* getContext() { return mGraphicsContext; };

	private:
		Context *mGraphicsContext;
};