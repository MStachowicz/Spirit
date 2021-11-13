class Context;

class Input
{
public:
    void initialise(Context* pContext);

    enum class InputType
    {
        Key_Escape,
        Unknown
    };
    void onInput(const InputType &pInput);  

    private:
         Context* mGraphicsContext; // GraphicsContext is required to perform 
};