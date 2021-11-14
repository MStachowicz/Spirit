// Context is an interface for specific graphics API's to implement.
// Renderer then uses the interface to call the correct implementation 
// based on the API selected at generation time.
class Context
{
public:
	virtual bool initialise()	= 0;
	virtual bool isClosing() 	= 0;
	
	virtual void close() 		= 0;
	virtual void clearWindow() 	= 0;
	virtual void swapBuffers() 	= 0;
	virtual void pollEvents() 	= 0;

	virtual void setClearColour(const float& pRed, const float& pGreen, const float& pBlue) = 0;
};