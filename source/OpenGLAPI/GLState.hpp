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

    int convert(const BlendFactorType& pBlendFactor);

	inline std::string toString(const DepthTestType& pDepthTestType)   { return depthTestTypes[util::toIndex(pDepthTestType)]; }
    inline std::string toString(const BufferDrawType& pBufferDrawType) { return bufferDrawTypes[util::toIndex(pBufferDrawType)]; }
    inline std::string toString(const BlendFactorType& pBlendFactorType) { return blendFactorTypes[util::toIndex(pBlendFactorType)]; }
}

// Tracks the current GLState and provides helpers to set global GL state using GlTypes.
class GLState
{
public:
    GLState();

	void toggleDepthTest(const bool& pDepthTest);
	void setDepthTestType(const GLType::DepthTestType& pType);

	// Pixels can be drawn using a function that blends the incoming (source) RGBA values with the RGBA values that are already in the frame buffer (the destination values).
	// Blending is default disabled in OpenGL.
    void toggleBlending(const bool& pBlend);
    void setBlendFunction(const GLType::BlendFactorType& pSourceFactor, const GLType::BlendFactorType& pDestinationFactor);

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

    int mBufferClearBitField; // Bit field sent to OpenGL clear buffers before next draw.
    std::array<float, 4> mWindowClearColour;
};