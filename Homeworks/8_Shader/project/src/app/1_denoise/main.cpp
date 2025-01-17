#pragma comment(lib,"ANN.lib") 
#include <UGL/UGL>
#include <UGM/UGM>

#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
<<<<<<< HEAD
=======

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

>>>>>>> 731ca7bc1d81ce390557433493ae319044dce0ec
#include "../../tool/Camera.h"
#include "../../tool/SimpleLoader.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <iostream>

#include <ANN.h>
#include <ANNx.h>	
#include <ANNperf.h>
#include <set>

using namespace Ubpa;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
gl::Texture2D loadTexture(char const* path);
gl::Texture2D genDisplacementmap(const SimpleLoader::OGLResources* resources);

void CacAdj(const SimpleLoader::OGLResources* resources);
std::vector<std::set<int>>adj;

void CacDelta(const SimpleLoader::OGLResources* resources, std::vector<vecf3>& delta,std::vector<float>& delat_proj);

void CacUniform(std::vector<float>& delta_proj);

/*void CacKnear(ANNpoint queryPt, ANNpointArray ptsArr, ANNidx* idxArr, ANNdist* distArr, ANNkd_tree* kdTree, int NUM, int K = 8)
{
	//std::cout << NUM << std::endl;
	//ANNkd_tree* kdTree = new ANNkd_tree(ptsArr, NUM,2 );
	//ANNbd_tree tree(ptsArr, NUM, 2);
	//tree.annkSearch(queryPt, K, idxArr, distArr);
}*/

void interpolation(float* displacementData, std::vector<int> index);

// settings
unsigned int scr_width = 800;
unsigned int scr_height = 600;
float displacement_bias = 0.f;
float displacement_scale = 1.f;
float displacement_lambda = 1.f;
bool have_denoise = true;

// camera
Camera camera(pointf3(0.0f, 0.0f, 3.0f));
float lastX = scr_width / 2.0f;
float lastY = scr_height / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(scr_width, scr_height, "HW8 - denoise", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    gl::Enable(gl::Capability::DepthTest);

    // build and compile our shader zprogram
    // ------------------------------------
    gl::Shader vs(gl::ShaderType::VertexShader, "../data/shaders/p3t2n3_denoise.vert");
    gl::Shader fs(gl::ShaderType::FragmentShader, "../data/shaders/light.frag");
    gl::Program program(&vs, &fs);
    rgbf ambient{ 0.2f,0.2f,0.2f };
    program.SetTex("albedo_texture", 0);
    program.SetTex("displacementmap", 1);
    program.SetVecf3("point_light_pos", { 0,5,0 });
    program.SetVecf3("point_light_radiance", { 100,100,100 });
    program.SetVecf3("ambient_irradiance", ambient);
    program.SetFloat("roughness", 0.5f );
    program.SetFloat("metalness", 0.f);

    // load model
    // ------------------------------------------------------------------
    auto spot = SimpleLoader::LoadObj("../data/models/spot_triangulated_good.obj", true);
    // world space positions of our cubes
    pointf3 instancePositions[] = {
        pointf3(0.0f,  0.0f,  0.0f),
        pointf3(2.0f,  5.0f, -15.0f),
        pointf3(-1.5f, -2.2f, -2.5f),
        pointf3(-3.8f, -2.0f, -12.3f),
        pointf3(2.4f, -0.4f, -3.5f),
        pointf3(-1.7f,  3.0f, -7.5f),
        pointf3(1.3f, -2.0f, -2.5f),
        pointf3(1.5f,  2.0f, -2.5f),
        pointf3(1.5f,  0.2f, -1.5f),
        pointf3(-1.3f,  1.0f, -1.5f)
    };

    // load and create a texture 
    // -------------------------
    gl::Texture2D spot_albedo = loadTexture("../data/textures/spot_albedo.png");

    gl::Texture2D displacementmap = genDisplacementmap(spot);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        gl::ClearColor({ ambient, 1.0f });
        gl::Clear(gl::BufferSelectBit::ColorBufferBit | gl::BufferSelectBit::DepthBufferBit); // also clear the depth buffer now!

        program.SetVecf3("camera_pos", camera.Position);

        // bind textures on corresponding texture units
        program.Active(0, &spot_albedo);
        program.Active(1, &displacementmap);

        // pass projection matrix to shader (note that in this case it could change every frame)
        transformf projection = transformf::perspective(to_radian(camera.Zoom), (float)scr_width / (float)scr_height, 0.1f, 100.f);
        program.SetMatf4("projection", projection);

        // camera/view transformation
        program.SetMatf4("view", camera.GetViewMatrix());
        program.SetFloat("displacement_bias", displacement_bias);
        program.SetFloat("displacement_scale", displacement_scale);
        program.SetFloat("displacement_lambda", displacement_lambda);
        program.SetBool("have_denoise", have_denoise);

        // render spots
        for (unsigned int i = 0; i < 10; i++)
        {
            // calculate the model matrix for each object and pass it to shader before drawing
            float angle = 20.0f * i + 10.f * (float)glfwGetTime();
            transformf model(instancePositions[i], quatf{ vecf3(1.0f, 0.3f, 0.5f), to_radian(angle) });
            program.SetMatf4("model", model);
            spot->va->Draw(&program);
        }

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    delete spot;

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera::Movement::DOWN, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        have_denoise = !have_denoise;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    gl::Viewport({ 0, 0 }, width, height);
    scr_width = width;
    scr_height = height;
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos); // reversed since y-coordinates go from bottom to top

    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    camera.ProcessMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

gl::Texture2D loadTexture(char const* path)
{
    gl::Texture2D tex;
    tex.SetWrapFilter(gl::WrapMode::Repeat, gl::WrapMode::Repeat, gl::MinFilter::Linear, gl::MagFilter::Linear);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    gl::PixelDataFormat c2f[4] = {
        gl::PixelDataFormat::Red,
        gl::PixelDataFormat::Rg,
        gl::PixelDataFormat::Rgb,
        gl::PixelDataFormat::Rgba
    };
    gl::PixelDataInternalFormat c2if[4] = {
        gl::PixelDataInternalFormat::Red,
        gl::PixelDataInternalFormat::Rg,
        gl::PixelDataInternalFormat::Rgb,
        gl::PixelDataInternalFormat::Rgba
    };
	
    if (data)
    {
        tex.SetImage(0, c2if[nrChannels - 1], width, height, c2f[nrChannels - 1], gl::PixelDataType::UnsignedByte, data);
        tex.GenerateMipmap();
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    return tex;
}

gl::Texture2D genDisplacementmap(const SimpleLoader::OGLResources* resources) {
    float* displacementData = new float[1024 * 1024];
	for (int i = 0; i < 1024 * 1024; i++)
		displacementData[i] = 0.0;
	std::vector<vecf3> nor(resources->positions.size(), 0.0f);
	CacAdj(resources);
	std::vector<vecf3> delta(resources->positions.size(), 0.0f);
	std::vector<float> delta_proj(resources->positions.size(), 0.0f);
	CacDelta(resources, delta, delta_proj);
	CacUniform(delta_proj);


	std::vector<int> index;

	auto p = resources->positions;

	for (int i = 0; i < p.size(); i++)
	{
		auto tex = resources->texcoords[i];
		auto u = tex[0], v = tex[1];
		int x = std::round(1024 * u-0.5);
		int y = std::round(1024 * v-0.5);

		//displacementData[x + y * 1024] = delta_proj[i];
		displacementData[x + y * 1024] = delta_proj[i];
		index.push_back(x + y * 1024);
	}
	interpolation(displacementData, index);
    // TODO: HW8 - 1_denoise | genDisplacementmap
    // 1. set displacementData with resources's positions, indices, normals, ...
    // 2. change global variable: displacement_bias, displacement_scale, displacement_lambda

    // ...

    gl::Texture2D displacementmap;
    displacementmap.SetImage(0, gl::PixelDataInternalFormat::Red, 1024, 1024, gl::PixelDataFormat::Red, gl::PixelDataType::Float, displacementData);
<<<<<<< HEAD
    //delete[] displacementData;
    displacementmap.SetWrapFilter(gl::WrapMode::Repeat, gl::WrapMode::Repeat,
        gl::MinFilter::Linear, gl::MagFilter::Linear);
	stbi_uc* stbi_data = new stbi_uc[1024 * 1024];
	for (size_t i = 0; i < 1024 * 1024; i++)
		stbi_data[i] = static_cast<stbi_uc>(std::clamp(displacementData[i] * 255.f, 0.f, 255.f));
	stbi_write_png("../data/1_denoise_displacement_map.png", 1024, 1024, 1, stbi_data, 1024);
	delete[] stbi_data;
	delete[] displacementData;
=======
    displacementmap.SetWrapFilter(gl::WrapMode::Repeat, gl::WrapMode::Repeat,
        gl::MinFilter::Linear, gl::MagFilter::Linear);
    stbi_uc* stbi_data = new stbi_uc[1024 * 1024];
    for (size_t i = 0; i < 1024 * 1024; i++)
        stbi_data[i] = static_cast<stbi_uc>(std::clamp(displacementData[i] * 255.f, 0.f, 255.f));
    stbi_write_png("../data/1_denoise_displacement_map.png", 1024, 1024, 1, stbi_data, 1024);
    delete[] stbi_data;
    delete[] displacementData;
>>>>>>> 731ca7bc1d81ce390557433493ae319044dce0ec
    return displacementmap;
}

void CacAdj(const SimpleLoader::OGLResources* resources)
{
	int n = resources->positions.size();

	for (int i = 0; i < n; i++)
	{
		std::set<int> point;
		adj.push_back(point);
	}

	auto ind = resources->indices;
	for (int i = 0; i < resources->indices.size(); i+=3)
	{
		int v1 = ind[i], v2 = ind[i + 1], v3 = ind[i + 2];
		adj[v1].insert(v2); adj[v1].insert(v3);
		adj[v2].insert(v1); adj[v2].insert(v3);
		adj[v3].insert(v1); adj[v3].insert(v2);
	}
	//for (int i = 0; i < n; i++)
		//std::cout << adj[i].size()<<" "<<i << std::endl;

	//for(auto t:adj[0])
		//std::cout << t << std::endl;
}

void CacDelta(const SimpleLoader::OGLResources* resources, std::vector<vecf3> &delta,std::vector<float> &delta_proj)
{
	auto points = resources->positions;
	auto nor = resources->normals;
	int n = points.size();
	//std::cout << nor[1];
	//std::vector<vecf3> delta(n,0.0f);

	for (int i = 0; i < n; i++)
	{
		pointf3 p = points[i];
		for (auto index : adj[i])
		{
			pointf3 pn = points[index];
			//std::cout << pn << std::endl << pn.cast_to<vecf3>() << std::endl<<delta[i]<<std::endl;
			//std::cout << delta[i] << std::endl;
			delta[i] += pn.cast_to<vecf3>();
		}
		//std::cout << nor[i] << std::endl;
		delta[i] = p.cast_to<vecf3>() - 1.0 / (adj[i].size()) * delta[i];
		//std::cout <<i<<" " <<adj[i].size()<< std::endl;
		//std::cout << delta[i] << std::endl << nor[i].cast_to<vecf3>() << std::endl << delta[i].dot(nor[i].cast_to<vecf3>());
		delta_proj[i] = delta[i].dot(nor[i].cast_to<vecf3>());
		//std::cout << i << " " << delta_proj[i] << std::endl;
	}

}

void CacUniform(std::vector<float>& delta_proj)
{
	float max = -10,min = 10;
	for (auto d : delta_proj)
	{
		if (d < min)
			min = d;
		if (d > max)
			max = d;
	}
	//std::cout << min << " " << max;

	for (int i = 0; i < delta_proj.size(); i++)
		delta_proj[i] = (delta_proj[i] - min) / (max - min);

	displacement_bias = min;
	displacement_scale = max - min;
	
}

void interpolation(float* displacementData, std::vector<int> index)
{
	int num = index.size();
	constexpr size_t K = 1;
	ANNpointArray ptsArr = annAllocPts(num, 2);
	
	for (int i = 0; i < num; i++)
	{
		ptsArr[i][0] = index[i] % 1024;
		ptsArr[i][1] = index[i] / 1024;
	}
	ANNkd_tree* kdTree = new ANNkd_tree(ptsArr, num, 2);
	for (int i = 0; i < 1024; i++)
	{
		for (int j = 0; j < 1024; j++)
		{
			//std::cout << displacementData[i + j * 1024] << std::endl;
			if (abs(displacementData[i + j * 1024]) <= 1e-6)
			{
				ANNidx idxArr[K];
				ANNdist distArr[K];
				ANNpoint queryPt = annAllocPt(2);
				queryPt[0] = i;
				queryPt[1] = j;
				kdTree->annkSearch(queryPt,1,idxArr,distArr);
				//displacementData[i + j * 1024] = 0.7;
				displacementData[i + j * 1024] = displacementData[index[idxArr[0]]];
				//std::cout << distArr[0] << std::endl;
				//std::cout << i<<" "<<j<<" "<<index[idxArr[0]]%1024<<" "<< index[idxArr[0]] / 1024 << " "<< displacementData[i + j * 1024] <<std::endl;
				
			}
		}
	}
	
}

