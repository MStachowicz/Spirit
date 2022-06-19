#include "Shader.hpp"

#include "FileSystem.hpp"
#include "Utility.hpp"
#include "Logger.hpp"

 Shader::Shader(const std::string& pName, GLState& pGLState)
	: mName(pName)
	, mSourcePath(File::GLSLShaderDirectory)
	, mTextureUnits(0)
{

	unsigned int vertexShader;
	{
		const std::string vertexShaderPath = mSourcePath + mName + ".vert";
		ZEPHYR_ASSERT(File::exists(vertexShaderPath), "Vertex shader does not exist at path {}", vertexShaderPath);
		vertexShader = pGLState.CreateShader(GLType::ShaderProgramType::Vertex);
		std::string source = File::readFromFile(vertexShaderPath);
		pGLState.ShaderSource(vertexShader, source);
		pGLState.CompileShader(vertexShader);
		scanForAttributes(source);
	}

	unsigned int fragmentShader;
	{
		const std::string fragmentShaderPath = mSourcePath + mName + ".frag";
		ZEPHYR_ASSERT(File::exists(fragmentShaderPath), "Fragment shader does not exist at path {}", fragmentShaderPath);
		fragmentShader = pGLState.CreateShader(GLType::ShaderProgramType::Fragment);
		std::string source = File::readFromFile(fragmentShaderPath);
		pGLState.ShaderSource(fragmentShader, source);
		pGLState.CompileShader(fragmentShader);
	}

	std::optional<unsigned int> geometryShader;
	{
		const std::string shaderPath = mSourcePath + mName + ".geom";
		if (File::exists(shaderPath))
		{
			geometryShader = pGLState.CreateShader(GLType::ShaderProgramType::Geometry);
			std::string source = File::readFromFile(shaderPath);
			pGLState.ShaderSource(geometryShader.value(), source);
			pGLState.CompileShader(geometryShader.value());
		}
	}

	{
		mHandle = pGLState.CreateProgram();
		pGLState.AttachShader(mHandle, vertexShader);
		pGLState.AttachShader(mHandle, fragmentShader);
		if (geometryShader.has_value())
			pGLState.AttachShader(mHandle, geometryShader.value());

		pGLState.LinkProgram(mHandle);
	}

	{ // Setup all the unform variables for the linked shader program using OpenGL program introspection
		{ // UniformBlock setup
			const int blockCount = pGLState.getActiveUniformBlockCount(mHandle);
			for (int blockIndex = 0; blockIndex < blockCount; blockIndex++)
			{
				mUniformBlocks.push_back(pGLState.getUniformBlock(mHandle, blockIndex));
				pGLState.bindUniformBlock(mUniformBlocks.back());
			}
		}

		{ // Loose UniformVariable setup
			const int uniformCount = pGLState.getActiveUniformCount(mHandle);
			for (int uniformIndex = 0; uniformIndex < uniformCount; ++uniformIndex)
			{
				GLData::UniformVariable uniform = pGLState.getUniformVariable(mHandle, uniformIndex);

				// Only add 'loose' uniforms, uniform block variables are handled in GLState::getUniformBlock
				if (uniform.mBlockIndex == -1)
					mUniformVariables.push_back(uniform);
			}
		}
	}

	{// Setup available Shader buffer objects.
		const int blockCount = pGLState.getShaderStorageBlockCount(mHandle);
		for (int blockIndex = 0; blockIndex < blockCount; blockIndex++)
		{
			mShaderBufferBlocks.push_back(pGLState.getShaderStorageBlock(mHandle, blockIndex));
			pGLState.bindShaderStorageBlock(mShaderBufferBlocks.back());
		}
	}

	{
		for (const auto &uniformBlock : mUniformBlocks)
		{
			for (const auto &uniformBlockVariable : uniformBlock.mVariables)
			{
				if (uniformBlockVariable.mType == GLType::DataType::Sampler2D || uniformBlockVariable.mType == GLType::DataType::SamplerCube)
					mTextureUnits++;
			}
		}
		for (const auto &uniformVariable : mUniformVariables)
		{
			if (uniformVariable.mType == GLType::DataType::Sampler2D || uniformVariable.mType == GLType::DataType::SamplerCube)
				mTextureUnits++;
		}
		ZEPHYR_ASSERT(mTextureUnits <= maxTextureUnits, "Texture units available must be below the max.");
	}

	{ // Setup the available texture units.
		// We have to tell OpenGL which texture unit each shader 'uniform sampler2D' belongs to by setting each sampler using glUniform1i.
		// We only have to set this once.
		if (mTextureUnits > 0)
		{
			use(pGLState);
			for (int j = 0; j < mTextureUnits; j++)
			{
				const std::string textureUniformName = "texture" + std::to_string(j);
				setUniform(pGLState, textureUniformName, j);
			}
		}
	}

	// Delete the shaders after linking as they're no longer needed
	pGLState.DeleteShader(vertexShader);
	pGLState.DeleteShader(fragmentShader);
	if (geometryShader.has_value())
		pGLState.DeleteShader(geometryShader.value());

	LOG_INFO("OpenGL::Shader '{}' loaded given ID: {}", mName, mHandle);
}

void Shader::scanForAttributes(const std::string& pSourceCode)
{
	if (mAttributes.find(Attribute::Position3D) == mAttributes.end())
		if(pSourceCode.find(getAttributeName(Attribute::Position3D)) != std::string::npos)
			mAttributes.insert(Attribute::Position3D);

	if (mAttributes.find(Attribute::Normal3D) == mAttributes.end())
		if (pSourceCode.find(getAttributeName(Attribute::Normal3D)) != std::string::npos)
			mAttributes.insert(Attribute::Normal3D);

	if (mAttributes.find(Attribute::ColourRGB) == mAttributes.end())
		if (pSourceCode.find(getAttributeName(Attribute::ColourRGB)) != std::string::npos)
			mAttributes.insert(Attribute::ColourRGB);

	if (mAttributes.find(Attribute::TextureCoordinate2D) == mAttributes.end())
		if (pSourceCode.find(getAttributeName(Attribute::TextureCoordinate2D)) != std::string::npos)
			mAttributes.insert(Attribute::TextureCoordinate2D);

	ZEPHYR_ASSERT(!mAttributes.empty() && mAttributes.size() <= util::toIndex(Attribute::Count), "{} is not a valid number of attributes for a shader.", mAttributes.size());
}

void Shader::use(GLState& pGLState) const
{
	pGLState.UseProgram(mHandle);
}

int Shader::getAttributeLocation(const Attribute& pAttribute)
{
	int location = static_cast<int>(util::toIndex(pAttribute));
	return location;

	// Non static version of getAttributeLocation, not necessary as attribute locations are
	// consistent across all Zephyr GLSL shaders.

	//const GLint location = glGetAttribLocation(mHandle, getAttributeName(pAttribute).c_str());
	//ZEPHYR_ASSERT(location != -1, "Failed to find the location of {} in shader {}.", getAttributeName(pAttribute).c_str(), mName);
	//return static_cast<int>(location);
}

int Shader::getAttributeComponentCount(const Attribute& pAttribute)
{
	switch (pAttribute)
	{
	case Attribute::Position3D:
		return 3; // X, Y and Z position components
	case Attribute::Normal3D:
		return 3; // X, Y and Z direction components
	case Attribute::ColourRGB:
		return 3; // Red, Green and Blue components
	case Attribute::TextureCoordinate2D:
		return 2; // X and Y components
	default:
		ZEPHYR_ASSERT(false, "Could not determine the size of the attribute pAttribute");
		return 0;
	}
}

std::string Shader::getAttributeName(const Attribute& pAttribute)
{
	if (pAttribute == Attribute::Position3D)
		return "VertexPosition";
	else if (pAttribute == Attribute::Normal3D)
		return "VertexNormal";
	else if (pAttribute == Attribute::ColourRGB)
		return  "VertexColour";
	else if (pAttribute == Attribute::TextureCoordinate2D)
		return "VertexTexCoord";
	else
		ZEPHYR_ASSERT(false, "Could not convert Shader::Attribute '{}' to an std::string", pAttribute);
		return "";
}
