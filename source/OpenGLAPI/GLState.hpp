#pragma once

#include "Utility.hpp"

#include "string"
#include "array"

// Wraps all the GL types into enums and provides helper functions to extract the values or string representations.
// All the enums come with a matching array to allow iterating over the enums in ImGui and converting to string by O(1) indexing.
namespace GLType
{
    enum BufferDrawType
    {
        Colour,
        Depth,
        Count
    };
    static inline const std::array<std::string, util::toIndex(GLType::BufferDrawType::Count)> bufferDrawTypes{
        "Colour",
        "Depth"};

    enum class DepthTestType
    {
        Always,
        Never,
        Less,
        Equal,
        NotEqual,
        Greater,
        LessEqual,
        GreaterEqual,
        Count
    };
    static inline const std::array<std::string, util::toIndex(GLType::DepthTestType::Count)> depthTestTypes{
        "Always",
        "Never",
        "Less",
        "Equal",
        "Not equal",
        "Greater than",
        "Less than or equal",
        "Greater than or equal"};

    enum class BlendFactorType
    {
        Zero,                      // Factor is equal to 0.
        One,                       // Factor is equal to 1.
        SourceColour,              // Factor is equal to the source colour vector.
        OneMinusSourceColour,      // Factor is equal to 1 minus the source colour vector.
        DestinationColour,         // Factor is equal to the destination colour vector
        OneMinusDestinationColour, // Factor is equal to 1 minus the destination colour vector
        SourceAlpha,               // Factor is equal to the alpha component of the source colour vector.
        OneMinusSourceAlpha,       // Factor is equal to 1 minus alpha of the source colour vector.
        DestinationAlpha,          // Factor is equal to the alpha component of the destination colour vector.
        OneMinusDestinationAlpha,  // Factor is equal to 1 minus alpha of the destination colour vector.
        ConstantColour,            // Factor is equal to the constant colour vector.
        OneMinusConstantColour,    // Factor is equal to 1 minus the constant colour vector.
        ConstantAlpha,             // Factor is equal to the alpha component of the constant colour vector.
        OneMinusConstantAlpha,     // Factor is equal to 1 minus alpha of the constant colour vector.
        Count
    };
    static inline const std::array<std::string, util::toIndex(GLType::BlendFactorType::Count)> blendFactorTypes{{
        "Zero",
        "One",
        "Source Colour",
        "One Minus Source Colour",
        "Destination Colour",
        "One Minus Destination Colour",
        "Source Alpha",
        "One Minus Source Alpha",
        "Destination Alpha",
        "One Minus Destination Alpha",
        "Constant Colour",
        "One Minus Constant Colour",
        "Constant Alpha",
        "One Minus Constant Alpha"}};

    enum class CullFacesType
    {
        Back,          // Culls only the back faces (Default OpenGL setting).
        Front,         // Culls only the front faces.
        FrontAndBack,  // Culls both the front and back faces.
        Count
    };
    static inline const std::array<std::string, util::toIndex(GLType::CullFacesType::Count)> cullFaceTypes {
        "Back",
        "Front",
        "Front and Back"};

    enum class FrontFaceOrientation
    {
        Clockwise,          // Clockwise polygons are identified as front-facing.
        CounterClockwise,   // Counter-clockwise polygons are identified as front-facing (Default OpenGL setting).
        Count
    };
    static inline const std::array<std::string, util::toIndex(GLType::FrontFaceOrientation::Count)> frontFaceOrientationTypes {
        "Clockwise",
        "CounterClockwise"};

    inline std::string toString(const DepthTestType& pDepthTestType)                { return depthTestTypes[util::toIndex(pDepthTestType)]; }
    inline std::string toString(const BufferDrawType& pBufferDrawType)              { return bufferDrawTypes[util::toIndex(pBufferDrawType)]; }
    inline std::string toString(const BlendFactorType& pBlendFactorType)            { return blendFactorTypes[util::toIndex(pBlendFactorType)]; }
    inline std::string toString(const CullFacesType& pCullFaceType)                 { return cullFaceTypes[util::toIndex(pCullFaceType)]; }
    inline std::string toString(const FrontFaceOrientation& pFrontFaceOrientation)  { return frontFaceOrientationTypes[util::toIndex(pFrontFaceOrientation)]; }

    int convert(const BlendFactorType &pBlendFactor);
}

// Wraps OpenGL data types that hold GPU data. Each type follows the same class structure:
// generate() - Creates a handle that data can be bound to.
// bind()     - Makes the data 'current'
// release()  - Deletes the data on the GPU freeing the handle (has to be called before destruction).
namespace GLData
{
    // Vertex Array Object (VAO)
    // Stores all of the state needed to supply vertex data. VAO::bind() needs to be called before setting the state using VBO's and EBO's.
    // It stores the format of the vertex data as well as the Buffer Objects (see below) providing the vertex data arrays.
    // Note: If you change any of the data in the buffers referenced by an existing VAO (VBO/EBO), those changes will be seen by users of the VAO.
	struct VAO
	{
		void generate();
		void bind() const;
		void release();
		unsigned int getHandle() { return mHandle; };

	private:
		bool mInitialised 		= false;
		unsigned int mHandle 	= 0;
	};
    // Vertex Buffer Object
    // Buffer storing per-vertex array data.
    struct VBO
	{
		void generate();
		void bind() const;
		void release();
	private:
		bool mInitialised 		= false;
		unsigned int mHandle 	= 0;
	};
    // Element Buffer Object
    // Buffer storing vertex index data defining which order to draw vertex data stored in a VBO.
    // EBO's are used only if a mesh uses Indexed drawing.
	struct EBO
	{
		void generate();
		void bind() const;
		void release();
		unsigned int getHandle() { return mHandle; };

	private:
		bool mInitialised 		= false;
		unsigned int mHandle 	= 0;
	};
    // OpenGL Texture object.
    // Represents a texture pushed to the GPU.
    struct Texture
	{
		void generate();
		void bind() const;
        void pushData(const int& pWidth, const int& pHeight, const int& pNumberOfChannels, const unsigned char* pData);
		void release();
		unsigned int getHandle() { return mHandle; };

	private:
		bool mInitialised 		= false;
		unsigned int mHandle 	= 0;
	};
}

// Tracks the current GLState and provides helpers to set global GL state using GlTypes.
class GLState
{
public:
    GLState();

	void toggleDepthTest(const bool& pDepthTest);
	void setDepthTestType(const GLType::DepthTestType& pType);

	// Specifies if objects with alpha values <1 should be blended using function set with setBlendFunction().
    void toggleBlending(const bool& pBlend);
    // Specifies how the RGBA factors of source and destination are blended to give the final pixel colour when encountering transparent objects.
    void setBlendFunction(const GLType::BlendFactorType& pSourceFactor, const GLType::BlendFactorType& pDestinationFactor);

    // Specifies if facets specified by setFrontFaceOrientation are candidates for culling.
    void toggleCullFaces(const bool& pCull);
    // Specifies which facets are candidates for culling.
    void setCullFacesType(const GLType::CullFacesType& pCullFaceType);
    // Specifies the orientation of front-facing polygons. Used to mark facets for culling.
    void setFrontFaceOrientation(const GLType::FrontFaceOrientation& pFrontFaceOrientation);

    // Specifies the red, green, blue, and alpha values to clear the color buffers. Values are clamped to the range 0-1.
    void setClearColour(const std::array<float, 4>& pColour);
	void clearBuffers();

    // Outputs the current GLState with options to change flags.
    void renderImGui();

private:
	bool mDepthTest;
	GLType::DepthTestType mDepthTestType;

    bool mBlend;
    GLType::BlendFactorType mSourceBlendFactor;
    GLType::BlendFactorType mDestinationBlendFactor;

    bool mCullFaces;
    GLType::CullFacesType mCullFacesType;
    GLType::FrontFaceOrientation mFrontFaceOrientation;

    int mBufferClearBitField; // Bit field sent to OpenGL clear buffers before next draw.
    std::array<float, 4> mWindowClearColour;
};