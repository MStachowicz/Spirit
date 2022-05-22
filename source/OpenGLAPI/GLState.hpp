#pragma once

#include "Utility.hpp"

#include "string"
#include "array"
#include "optional"

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

    // Polygon rasterization mode
    // Vertices are marked as boundary/non-boundary with an edge flag generated internally by OpenGL when it decomposes triangle stips and fans.
    enum class PolygonMode
    {
        Point, // Polygon vertices that are marked as the start of a boundary edge are drawn as points. Point attributes such as GL_POINT_SIZE and GL_POINT_SMOOTH control the rasterization of the points.
        Line,  // Boundary edges of the polygon are drawn as line segments. Line attributes such as GL_LINE_WIDTH and GL_LINE_SMOOTH control the rasterization of the lines.
        Fill,  // The interior of the polygon is filled. Polygon attributes such as GL_POLYGON_SMOOTH control the rasterization of the polygon. (Default OpenGL setting).
        Count
    };
    static inline const std::array<std::string, util::toIndex(PolygonMode::Count)> polygonModeTypes{
        "Point",
        "Line",
        "Fill"};

    // Interpretation scheme used to determine what a stream of vertices represents when being rendered.
    enum class PrimitiveMode
    {
        Points,
        LineStrip,
        LineLoop,
        Lines,
        LineStripAdjacency,
        LinesAdjacency,
        TriangleStrip,
        TriangleFan,
        Triangles,
        TriangleStripAdjacency,
        TrianglesAdjacency,
        Patches,
        Count
    };
    static inline const std::array<std::string, util::toIndex(PrimitiveMode::Count)> primitiveModeTypes{
        "Points"
        "LineStrip"
        "LineLoop"
        "Lines"
        "LineStripAdjacency"
        "LinesAdjacency"
        "TriangleStrip"
        "TriangleFan"
        "Triangles"
        "TriangleStripAdjacency"
        "TrianglesAdjacency"
        "Patches"};

    enum class FramebufferTarget
    {
        DrawFramebuffer,
        ReadFramebuffer,
        Framebuffer,
        Count
    };
    static inline const std::array<std::string, util::toIndex(FramebufferTarget::Count)> FrameBufferTargetTypes{
        "DrawFramebuffer",
        "ReadFramebuffer",
        "Framebuffer"};

    inline std::string toString(const DepthTestType& pDepthTestType)                { return depthTestTypes[util::toIndex(pDepthTestType)]; }
    inline std::string toString(const BufferDrawType& pBufferDrawType)              { return bufferDrawTypes[util::toIndex(pBufferDrawType)]; }
    inline std::string toString(const BlendFactorType& pBlendFactorType)            { return blendFactorTypes[util::toIndex(pBlendFactorType)]; }
    inline std::string toString(const CullFacesType& pCullFacesType)                { return cullFaceTypes[util::toIndex(pCullFacesType)]; }
    inline std::string toString(const FrontFaceOrientation& pFrontFaceOrientation)  { return frontFaceOrientationTypes[util::toIndex(pFrontFaceOrientation)]; }
    inline std::string toString(const PolygonMode& pPolygonMode)                    { return polygonModeTypes[util::toIndex(pPolygonMode)]; }
    inline std::string toString(const PrimitiveMode& pPrimitiveMode)                { return primitiveModeTypes[util::toIndex(pPrimitiveMode)]; }
    inline std::string toString(const FramebufferTarget& pFramebufferTarget)        { return FrameBufferTargetTypes[util::toIndex(pFramebufferTarget)]; }

    int convert(const DepthTestType& pDepthTestType);
    int convert(const BlendFactorType& pBlendFactorType);
    int convert(const CullFacesType& pCullFacesType);
    int convert(const FrontFaceOrientation& pFrontFaceOrientation);
    int convert(const PolygonMode& pPolygonMode);
    int convert(const PrimitiveMode& pPrimitiveMode);
    int convert(const FramebufferTarget& pFramebufferTarget);
}

class GLState;
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
        void pushData(const std::vector<float>& pData, const int& attributeIndex, const int& attributeSize);
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
        void pushData(const std::vector<int>& pIndexData);
		unsigned int getHandle() { return mHandle; };
	private:
		bool mInitialised 		= false;
		unsigned int mHandle 	= 0;
	};

    struct UBO
    {
		void generate();
		void bind() const;
		void release();
        void pushData(const int& pSize, const int& pUniformIndex);
		unsigned int getHandle() { return mHandle; };
	private:
		bool mInitialised 		= false;
		unsigned int mHandle 	= 0;
    };
    // OpenGL Texture object.
    // Represents a texture pushed to the GPU.
    struct Texture
	{
        friend struct FBO;

        enum class Type
        {
            Texture2D, // Texture target: GL_TEXTURE_2D
            CubeMap, // Texture target: GL_TEXTURE_CUBE_MAP
            None
        };
        Texture(const Type& pType) : mType(pType) {}
        Texture() = default;

		void generate();
		void bind() const;
        // Pushes the texture data using glTexImage2D.
        // If pCubeMapIndexOffset is supplied the data is pushed to GL_TEXTURE_CUBE_MAP at index GL_TEXTURE_CUBE_MAP_POSITIVE_X (0) + offset to GL_TEXTURE_CUBE_MAP_NEGATIVE_Z (5).
        void pushData(const int& pWidth, const int& pHeight, const int& pNumberOfChannels, const unsigned char* pData, const int& pCubeMapIndexOffset = -1);
		void release();
		unsigned int getHandle() { return mHandle; };
	private:
		bool mInitialised 		= false;
		unsigned int mHandle 	= 0;
        Type mType              = Type::None;
	};
    // Render Buffer Object
    // RBO's contain images optimized for use as render targets, and are the logical choice when you do not need to sample (i.e. in a post-pass shader) from the produced image.
    // If you need to resample (such as when reading depth back in a second shader pass), use Texture instead.
    // RBO's are created and used specifically with Framebuffer Objects (FBO's).
    struct RBO
    {
        void generate();
		void bind() const;
        void release();
		unsigned int getHandle() { return mHandle; };
	private:
		bool mInitialised 		= false;
		unsigned int mHandle 	= 0;
    };
    // Framebuffer object.
    // Allows creation of user-defined framebuffers that can be rendered to without disturbing the main screen.
    struct FBO
    {
        void generate();
		void bind(GLState& pGLState) const;
        void release();
		unsigned int getHandle() { return mHandle; };
        Texture& getColourTexture();
        void clearBuffers();

        void resize(const int& pWidth, const int& pHeight, GLState& pGLState);
        void attachColourBuffer(const int& pWidth, const int& pHeight, GLState& pGLState);
        void detachColourBuffer();
        void attachDepthBuffer(const int& pWidth, const int& pHeight, GLState& pGLState);
        void detachDepthBuffer();
	private:
        std::optional<Texture> mColourAttachment    = std::nullopt;
        std::optional<RBO> mDepthAttachment         = std::nullopt;

		bool mInitialised 		= false;
		unsigned int mHandle 	= 0;
        int mBufferClearBitField = 0; // Bit field sent to OpenGL clear buffers before next draw.
    };
}

// Tracks the current GLState and provides helpers to set global GL state using GlTypes.
class GLState
{
public:
    GLState();

    // Copies the state of pOther and sets all the discrepant GL states.
    GLState& operator=(const GLState& pOther);
    // Checks if GLState matches what OpenGL state machine is set to.
    bool validateState();

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
    // Controls the interpretation of polygons for rasterization.
    // pPolygonMode: Specifies how polygons will be rasterized.
    // Affects only the final rasterization of polygons - a polygon's vertices are lit and the polygon is clipped/culled before these modes are applied.
    void setPolygonMode(const GLType::PolygonMode& pPolygonMode);

    // Selects active texture unit subsequent texture state calls will affect. The number of texture units an implementation supports is implementation dependent, but must be at least 80.
    void setActiveTextureUnit(const int& pUnitPosition);

    // Render primitives from array data
    // pPrimitiveMode: What kind of primitives to render.
    // pCount: Number of elements to be rendered.
    // Specifies multiple geometric primitives with very few subroutine calls.
    // Instead of passing each individual vertex, normal, texture coordinate... you can pre-specify separate arrays of vertices, normals, and so on, and use them to construct a sequence of primitives with a single call to glDrawElements.
    void drawElements(const GLType::PrimitiveMode& pPrimitiveMode, const int& pCount);
    // Render primitives from array data
    // pPrimitiveMode: What kind of primitives to render.
    // pCount: Number of indices to be rendered.
    // Specifies multiple geometric primitives with very few subroutine calls.
    // Instead of passing each individual vertex, normal, texture coordinate... you can pre-specify separate arrays of vertices, normals, and so on, and use them to construct a sequence of primitives with a single call to glDrawElements.
    void drawArrays(const GLType::PrimitiveMode& pPrimitiveMode, const int& pCount);

    // Bind a FBO to a framebuffer target
    void bindFramebuffer(const GLType::FramebufferTarget& pFramebufferTargetType, const unsigned int& pFBOHandle);
    // Unbind any current framebuffer, this binds the default OpenGL framebuffer.
    void unbindFramebuffer();
    void checkFramebufferBufferComplete();

    // Set the global viewport
    // Specify the width and height of the viewport. When a context is first attached to a window, width and height are set to the dimensions of that window.
    // The viewport specifies the affine transformation of x and y from normalized device coordinates to window coordinates.
    // Let x nd y nd be normalized device coordinates. Then the window coordinates x w y w are computed as follows:
    // x w = x nd + 1 ⁢ width 2 + x
    // y w = y nd + 1 ⁢ height 2 + y
    void setViewport(const int& pWidth, const int& pHeight);

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

    std::array<float, 4> mWindowClearColour;
    GLType::PolygonMode mPolygonMode;

    int mActiveTextureUnit;
    unsigned int mActiveFramebuffer;

    // Index data:
    // 0: Position X (0,0 = bottom-left)
    // 1: Position Y (0,0 = bottom-left)
    // 2: Size X
    // 3: Size Y
    std::array<int, 4> mViewport;
};