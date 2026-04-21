#include <main_state.h>
#include <glad/glad.h>
#include <math.h>


#include <rafgl.h>

#include <game_constants.h>

#include <main_state.h>
#include <glad/glad.h>
#include <math.h>
#include <rafgl.h>
#include <game_constants.h>

static GLuint terrain_vao, terrain_vbo, terrain_ebo;
static unsigned int terrain_vertex_count;
rafgl_texture_t rock_tex, lava_tex, rock_normal_tex, lava_normal_tex;
int line_vertex_count;
float scaleY = 5.0f;
vec3_t camera_position = vec3m(0.0f, 3.0f, 3.5f);

#include <stdlib.h> // za rand()

// za random generisanje po sin i cos

float random_float(float min, float max)
{
    return min + (rand() / (float)RAND_MAX) * (max - min);
}


vec3_t v3_lerp(vec3_t a, vec3_t b, float t)
{
    return vec3(
        a.x + t * (b.x - a.x),
        a.y + t * (b.y - a.y),
        a.z + t * (b.z - a.z)
    );
}

void load_tex(rafgl_texture_t *tex, const char *path)
{
    rafgl_raster_t raster;
    rafgl_raster_load_from_image(&raster, path);
    rafgl_texture_init(tex);
    rafgl_texture_load_from_raster(tex, &raster);

    glBindTexture(GL_TEXTURE_2D, tex->tex_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);

}

typedef struct _vertex_t
{
    vec3_t position;
    vec3_t colour;
    float alpha;
    float u, v;
    vec3_t normal;
    vec3_t tangent;  // dodajemo
    vec3_t bitangent;// dodajemo

} vertex_t;


typedef struct {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    int index_count;
    vec3_t center;
} customTerrain_t;


vertex_t vertex(vec3_t pos, vec3_t col, float alpha, float u, float v, vec3_t normal)
{
    vertex_t vert;
    vert.position = pos;
    vert.colour   = col;
    vert.alpha    = alpha;
    vert.u        = u;
    vert.v        = v;
    vert.normal   = normal;
    return vert;
}

static void compute_tangent_bitangent(
    vec3_t p0, vec3_t p1, vec3_t p2,
    float u0, float v0, float u1, float v1, float u2, float v2,
    vec3_t *face_normal,
    vec3_t *face_tangent,
    vec3_t *face_bitangent)
{
    //face normal
    vec3_t e1 = v3_sub(p1, p0);
    vec3_t e2 = v3_sub(p2, p0);
    *face_normal = v3_cross(e1, e2);

    // tangent bitangent
    float du1 = (u1 - u0), dv1 = (v1 - v0);
    float du2 = (u2 - u0), dv2 = (v2 - v0);

    float denom = (du1 * dv2 - du2 * dv1);
    if(fabs(denom) < 1e-8) {
        *face_tangent   = vec3(1.0f, 0.0f, 0.0f);
        *face_bitangent = vec3(0.0f, 1.0f, 0.0f);
        return;
    }
    float f = 1.0f / denom;

    vec3_t t = vec3(
        f * ( dv2*e1.x - dv1*e2.x ),
        f * ( dv2*e1.y - dv1*e2.y ),
        f * ( dv2*e1.z - dv1*e2.z )
    );
    vec3_t b = vec3(
        f * (-du2*e1.x + du1*e2.x ),
        f * (-du2*e1.y + du1*e2.y ),
        f * (-du2*e1.z + du1*e2.z )
    );

    *face_tangent   = t;
    *face_bitangent = b;
}

static GLuint shader_program_id, uni_M, uni_VP, uni_object_colour, uni_light_colour, uni_light_direction, uni_ambient;
static rafgl_meshPUN_t monkey;
//static customTerrain_t mesh;

vec3_t object_colour = RAFGL_BLUE;
vec3_t light_colour  = RAFGL_WHITE;
vec3_t light_direction = vec3m(-1.0f, -1.0f, -1.0f);
vec3_t ambient = RAFGL_GRAYX(0.9f);




static rafgl_meshPUN_t terrain_mesh;
static customTerrain_t terrain_mesh_part [4];
static customTerrain_t terrain_mesh_part_low [4];

static rafgl_raster_t heightmap_raster;


/*
rafgl_pixel_rgb_t sample = pixel_at_m(heightmap_raster,x,y);
            int r = sample.r;
            float h00 = (r / 255.0f) * scaleY;
            float h10 = (r / 255.0f) * scaleY;
            float h01 = (r / 255.0f) * scaleY;
            float h11 = (r / 255.0f) * scaleY;*/

int total_indices;
float res_const = 1.0f;
void build_terrain_from_heightmapPart(customTerrain_t *mesh, rafgl_raster_t *raster, int x0, int y0, int sub_width, int sub_height)
{
    int width = raster->width;
    int height = raster->height;
    int fullWidth = width, fullHeight = height;
    width  = sub_width;
    height = sub_height;




    float scaleXZ = 0.1f;

    float global_offsetX = (fullWidth  - 1) * scaleXZ / 2.0f;
    float global_offsetZ = (fullHeight - 1) * scaleXZ / 2.0f;
    //float scaleY = 3.0f;
    float offsetX = (width - 1+x0)  * scaleXZ / 2.0f;
    float offsetZ = (height - 1+y0) * scaleXZ / 2.0f;



    float centerPixelX = x0 + (sub_width - 1) / 2.0f;
    float centerPixelY = y0 + (sub_height - 1) / 2.0f;

    float chunk_center_x = centerPixelX * scaleXZ - global_offsetX;
    float chunk_center_z = centerPixelY * scaleXZ - global_offsetZ;
    float chunk_center_y = 0;

    mesh->center = vec3(chunk_center_x, chunk_center_y, chunk_center_z);



    //mallocuj za w*h
    int vertex_count = width * height;
    vec3_t *vertex_positions = (vec3_t *) malloc(vertex_count * sizeof(vec3_t));

    int vertex_index = 0;
    for(int yy = 0; yy < height; yy++)
    {
        for(int xx = 0; xx < width; xx++)
        {
            int globalX = x0 + xx;
            int globalY = y0 + yy;

            rafgl_pixel_rgb_t sample = pixel_at_m(heightmap_raster, globalX, globalY);
            float h = (sample.r / 255.0f) * scaleY;

            float worldX = (globalX * scaleXZ) - global_offsetX;
            float worldZ = (globalY * scaleXZ) - global_offsetZ;

            vertex_positions[vertex_index++] = vec3(worldX, h, worldZ);
        }
    }

    vertex_t *triangle_vertices = (vertex_t *) malloc(vertex_count * sizeof(vertex_t));
    for (int v = 0; v < vertex_count; v++)
    {
        triangle_vertices[v].position = vertex_positions[v];
        triangle_vertices[v].colour   = vec3m(1.0f, 1.0f, 1.0f);

        int i = v % width;
        int j = v / width;
        triangle_vertices[v].u = (float)i / (float)(width  - 1);
        triangle_vertices[v].v = (float)j / (float)(height - 1);

        triangle_vertices[v].normal    = vec3(0.0f, 0.0f, 0.0f);
        triangle_vertices[v].tangent   = vec3(0.0f, 0.0f, 0.0f);
        triangle_vertices[v].bitangent = vec3(0.0f, 0.0f, 0.0f);
    }

    int num_cells_x = (width  - 1);
    int num_cells_y = (height - 1);
    int total_triangles = num_cells_x * num_cells_y * 2 / res_const;
    int total_indices_local   = total_triangles * 3;

    GLuint *indices = (GLuint *) malloc(total_indices_local * sizeof(GLuint));

    int index_cursor = 0;
    for (int y = 0; y < num_cells_y-res_const; y+=res_const)
    {
        for (int x = 0; x < num_cells_x-res_const; x+=res_const)
        {
            //deli se kvadrta
            int i0 =  y      * width +  x;
            int i1 =  y      * width + (x + res_const);
            int i2 = (y + res_const) * width +  x;
            int i3 = (y + res_const) * width + (x + res_const);

            //2 trougla
            indices[index_cursor++] = i0;
            indices[index_cursor++] = i2;
            indices[index_cursor++] = i1;


            indices[index_cursor++] = i1;
            indices[index_cursor++] = i2;
            indices[index_cursor++] = i3;
        }
    }


    for(int i = 0; i < total_indices_local; i += 3)
{
int i0 = indices[i + 0];
int i1 = indices[i + 1];
int i2 = indices[i + 2];
vertex_t *v0 = &triangle_vertices[i0];
vertex_t *v1 = &triangle_vertices[i1];
vertex_t *v2 = &triangle_vertices[i2];

vec3_t p0 = v0->position;
vec3_t p1 = v1->position;
vec3_t p2 = v2->position;

// UV
float u0 = v0->u, v0_ = v0->v;
float u1 = v1->u, v1_ = v1->v;
float u2 = v2->u, v2_ = v2->v;

vec3_t fn, ft, fb;
compute_tangent_bitangent(p0, p1, p2,
                          u0, v0_, u1, v1_, u2, v2_,
                          &fn, &ft, &fb);

//akumulira
v0->normal    = v3_add(v0->normal, fn);
v1->normal    = v3_add(v1->normal, fn);
v2->normal    = v3_add(v2->normal, fn);

v0->tangent   = v3_add(v0->tangent, ft);
v1->tangent   = v3_add(v1->tangent, ft);
v2->tangent   = v3_add(v2->tangent, ft);

v0->bitangent = v3_add(v0->bitangent, fb);
v1->bitangent = v3_add(v1->bitangent, fb);
v2->bitangent = v3_add(v2->bitangent, fb);
}

for(int i = 0; i < vertex_count; i++)
{
triangle_vertices[i].normal    = v3_norm(triangle_vertices[i].normal);
triangle_vertices[i].tangent   = v3_norm(triangle_vertices[i].tangent);
triangle_vertices[i].bitangent = v3_norm(triangle_vertices[i].bitangent);
}


    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ebo);

    // VAO
    glBindVertexArray(mesh->vao);

    // VBO
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(vertex_t),
                 triangle_vertices, GL_STATIC_DRAW);

    // EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 total_indices_local * sizeof(GLuint),
                 indices, GL_STATIC_DRAW);

    // poz- location = 0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertex_t),
                          (void*)offsetof(vertex_t, position));


    /*int scale = 10;
    // boja: location = 1
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE,
                          sizeof(int),
                          (void*)offsetof(int, scale));*/

    //uv koord - location = 2
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                          sizeof(vertex_t),
                          (void*)offsetof(vertex_t, u));


         //TBNTBNTBNTBNTBTNBTNTBNBTNTBNTNTB
        glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertex_t),
                          (void*)offsetof(vertex_t, normal));

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertex_t),
                          (void*)offsetof(vertex_t, tangent));

    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertex_t),
                          (void*)offsetof(vertex_t, bitangent));





    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    mesh->index_count = total_indices_local;

    free(vertex_positions);
    free(triangle_vertices);
    free(indices);
}

void build_terrain_from_heightmap(rafgl_meshPUN_t *mesh, rafgl_raster_t *raster)
{
    int width = raster->width;
    int height = raster->height;

    float scaleXZ = 0.1f;
    //float scaleY = 3.0f;
    float offsetX = (width - 1)  * scaleXZ / 2.0f;
    float offsetZ = (height - 1) * scaleXZ / 2.0f;

    //mallocuj za w*h
    int vertex_count = width * height;
    vec3_t *vertex_positions = (vec3_t *) malloc(vertex_count * sizeof(vec3_t));

    int vertex_index = 0;
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            rafgl_pixel_rgb_t sample = pixel_at_m(heightmap_raster, x, y);
            //dovoljno po r INTENZITET
            float h = (sample.r / 255.0f) * scaleY;

            vertex_positions[vertex_index++] = vec3(
                x * scaleXZ - offsetX,
                h,//(visina)
                y * scaleXZ - offsetZ
            );
        }
    }

    vertex_t *triangle_vertices = (vertex_t *) malloc(vertex_count * sizeof(vertex_t));
    for (int i = 0; i < vertex_count; i++)
    {
        triangle_vertices[i].position = vertex_positions[i];
        triangle_vertices[i].colour   = vec3m(1.0f, 1.0f, 1.0f);
        float x = vertex_positions[i].x;
        float z = vertex_positions[i].z;
        triangle_vertices[i].u = (float)x / (float)(width - 1);
        triangle_vertices[i].v = (float)z / (float)(height - 1);

        //TBNTBNTBNTBN
        triangle_vertices[i].normal    = vec3(0.0f, 0.0f, 0.0f);
        triangle_vertices[i].tangent   = vec3(0.0f, 0.0f, 0.0f);
        triangle_vertices[i].bitangent = vec3(0.0f, 0.0f, 0.0f);
    }

    int num_cells_x = (width  - 1);
    int num_cells_y = (height - 1);
    int total_triangles = num_cells_x * num_cells_y * 2 / res_const;
    total_indices   = total_triangles * 3;

    GLuint *indices = (GLuint *) malloc(total_indices * sizeof(GLuint));

    int index_cursor = 0;
    for (int y = 0; y < num_cells_y-res_const; y+=res_const)
    {
        for (int x = 0; x < num_cells_x-res_const; x+=res_const)
        {
            int i0 =  y      * width +  x;
            int i1 =  y      * width + (x + res_const);
            int i2 = (y + res_const) * width +  x;
            int i3 = (y + res_const) * width + (x + res_const);


            // 1. trougao
            indices[index_cursor++] = i0;
            indices[index_cursor++] = i2;
            indices[index_cursor++] = i1;

            // 2. trougao
            indices[index_cursor++] = i1;
            indices[index_cursor++] = i2;
            indices[index_cursor++] = i3;
        }
    }



    for(int i = 0; i < total_indices; i += 3)
{
int i0 = indices[i + 0];
int i1 = indices[i + 1];
int i2 = indices[i + 2];
vertex_t *v0 = &triangle_vertices[i0];
vertex_t *v1 = &triangle_vertices[i1];

vertex_t *v2 = &triangle_vertices[i2];

vec3_t p0 = v0->position;
vec3_t p1 = v1->position;
vec3_t p2 = v2->position;

//uv
float u0 = v0->u, v0_ = v0->v;
float u1 = v1->u, v1_ = v1->v;
float u2 = v2->u, v2_ = v2->v;

// racuna normal/tangent/bitangent
vec3_t fn, ft, fb;
compute_tangent_bitangent(p0, p1, p2,
                          u0, v0_, u1, v1_, u2, v2_,
                          &fn, &ft, &fb);

// akumulira u svaki vertex
v0->normal    = v3_add(v0->normal, fn);
v1->normal    = v3_add(v1->normal, fn);
v2->normal    = v3_add(v2->normal, fn);

v0->tangent   = v3_add(v0->tangent, ft);
v1->tangent   = v3_add(v1->tangent, ft);
v2->tangent   = v3_add(v2->tangent, ft);

v0->bitangent = v3_add(v0->bitangent, fb);
v1->bitangent = v3_add(v1->bitangent, fb);
v2->bitangent = v3_add(v2->bitangent, fb);
}

for(int i = 0; i < vertex_count; i++)
{
triangle_vertices[i].normal    = v3_norm(triangle_vertices[i].normal);
triangle_vertices[i].tangent   = v3_norm(triangle_vertices[i].tangent);
triangle_vertices[i].bitangent = v3_norm(triangle_vertices[i].bitangent);
}


    glGenVertexArrays(1, &terrain_vao);
    glGenBuffers(1, &terrain_vbo);
    glGenBuffers(1, &terrain_ebo);

    // VAO
    glBindVertexArray(terrain_vao);

    // VBO - za verteksa
    glBindBuffer(GL_ARRAY_BUFFER, terrain_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 vertex_count * sizeof(vertex_t),
                 triangle_vertices,
                 GL_STATIC_DRAW);

    // EBO - za indekse
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrain_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 total_indices * sizeof(GLuint),
                 indices,
                 GL_STATIC_DRAW);

    // pozicija: location = 0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertex_t),
                          (void*)offsetof(vertex_t, position));
    // boja: location = 1
    /*glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertex_t),
                          (void*)offsetof(vertex_t, colour));*/

    // UV koord location = 2
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                          sizeof(vertex_t),
                          (void*)offsetof(vertex_t, u));


         //TBNTBNTBNTBNTBTNBTNTBNBTNTBNTNTB
        glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertex_t),
                          (void*)offsetof(vertex_t, normal));

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertex_t),
                          (void*)offsetof(vertex_t, tangent));

    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertex_t),
                          (void*)offsetof(vertex_t, bitangent));





    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //free
    free(vertex_positions);
    free(triangle_vertices);
    free(indices);
}









void main_state_init(GLFWwindow *window, void *args, int width, int height)
{
    rafgl_log_fps(RAFGL_TRUE);
    glDisable(GL_CULL_FACE);

    customTerrain_t mesh = {0};
    load_tex(&rock_tex, "res/images/rock.png");// ROCK
    load_tex(&lava_tex, "res/images/lava.jpg");// LAVA
    load_tex(&rock_normal_tex, "res/images/rockNormalMap.png");
    load_tex(&lava_normal_tex, "res/images/lavaNormalMap.jpg");


    // ucitaj heightmap
    rafgl_raster_load_from_image(&heightmap_raster, "res/images/heightmap.jpg");
    //rafgl_raster_load_from_image(&heightmap_raster, "res/images/heightmap2.png");

    // Kreiraj teren
    build_terrain_from_heightmap(&terrain_mesh, &heightmap_raster);



    int sub_width  = 102;
    int sub_height = 96;
    // gornje levo (pod-terrain 0)
    build_terrain_from_heightmapPart(&terrain_mesh_part[0],
                            &heightmap_raster,
                            0, 0,
                            sub_width, sub_height);

    // gornje desno (pod-terrain 1)
    build_terrain_from_heightmapPart(&terrain_mesh_part[1],
                            &heightmap_raster,
                            102-6, 0,
                            sub_width, sub_height);

    //res_const = 3;

    // donje levo (pod-terrain 2)
    build_terrain_from_heightmapPart(&terrain_mesh_part[2],
                            &heightmap_raster,
                            0, 96-6,
                            sub_width, sub_height);

    // donje desno (pod-terrain 3)
    build_terrain_from_heightmapPart(&terrain_mesh_part[3],
                            &heightmap_raster,
                            102-6, 96-6,
                            sub_width, sub_height);


    res_const = 7;

    //LOW RES
    // gornje levo (pod-terrain 0)
    build_terrain_from_heightmapPart(&terrain_mesh_part_low[0],
                            &heightmap_raster,
                            0, 0,
                            sub_width, sub_height);

    // gornje desno (pod-terrain 1)
    build_terrain_from_heightmapPart(&terrain_mesh_part_low[1],
                            &heightmap_raster,
                            102-6, 0,
                            sub_width, sub_height);

    //res_const = 3;

    // donje levo (pod-terrain 2)
    build_terrain_from_heightmapPart(&terrain_mesh_part_low[2],
                            &heightmap_raster,
                            0, 96-6,
                            sub_width, sub_height);

    // donje desno (pod-terrain 3)
    build_terrain_from_heightmapPart(&terrain_mesh_part_low[3],
                            &heightmap_raster,
                            102-6, 96-6,
                            sub_width, sub_height);

    res_const = 1;


    // ucitava majmuna
    rafgl_meshPUN_init(&monkey);
    rafgl_meshPUN_load_from_OBJ(&monkey, "res/models/monkey.obj");

    // second shader
    shader_program_id = rafgl_program_create_from_name("second_shader");
    uni_M = glGetUniformLocation(shader_program_id, "uni_M");
    uni_VP = glGetUniformLocation(shader_program_id, "uni_VP");
    /*uni_object_colour = glGetUniformLocation(shader_program_id, "uni_object_colour");
    uni_light_colour = glGetUniformLocation(shader_program_id, "uni_light_colour");
    uni_light_direction = glGetUniformLocation(shader_program_id, "uni_light_direction");
    uni_ambient = glGetUniformLocation(shader_program_id, "uni_ambient");*/

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    light_direction = v3_norm(light_direction);

    //glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}




mat4_t model, view, projection, view_projection;

/* field of view */
float fov = 75.0f;


vec3_t camera_target = vec3m(0.0f, 0.0f, 0.0f);
vec3_t camera_up = vec3m(0.0f, 1.0f, 0.0f);
vec3_t aim_dir = vec3m(0.0f, 0.0f, -1.0f);

float camera_angle = -M_PIf * 0.5f;
float angle_speed = 0.2f * M_PIf;
float move_speed = 2.4f;

float hoffset = 0;

void v3show(vec3_t v)
{
    printf("(%.2f %.2f %.2f)\n", v.x, v.y, v.z);
}

float time = 0.0f;
int reshow_cursor_flag = 0;
int last_lmb = 0;

int renderLines = 0;
int drawFullTerrain = 0;
void main_state_update(GLFWwindow *window, float delta_time, rafgl_game_data_t *game_data, void *args)
{

    time += delta_time;


    if(game_data->is_lmb_down)
    {

        if(reshow_cursor_flag == 0)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        }

        float ydelta = game_data->mouse_pos_y - game_data->raster_height / 2;
        float xdelta = game_data->mouse_pos_x - game_data->raster_width / 2;

        if(!last_lmb)
        {
            ydelta = 0;
            xdelta = 0;
        }

        hoffset -= ydelta / game_data->raster_height;
        camera_angle += xdelta / game_data->raster_width;

        glfwSetCursorPos(window, game_data->raster_width / 2, game_data->raster_height / 2);
        reshow_cursor_flag = 1;
    }
    else if(reshow_cursor_flag)
    {
        reshow_cursor_flag = 0;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    last_lmb = game_data->is_lmb_down;

    aim_dir = v3_norm(vec3(cosf(camera_angle), hoffset, sinf(camera_angle)));

    if(game_data->keys_down['W']) camera_position = v3_add(camera_position, v3_muls(aim_dir, move_speed * delta_time));
    if(game_data->keys_down['S']) camera_position = v3_add(camera_position, v3_muls(aim_dir, -move_speed * delta_time));

    vec3_t right = v3_cross(aim_dir, vec3(0.0f, 1.0f, 0.0f));
    if(game_data->keys_down['D']) camera_position = v3_add(camera_position, v3_muls(right, move_speed * delta_time));
    if(game_data->keys_down['A']) camera_position = v3_add(camera_position, v3_muls(right, -move_speed * delta_time));

    if(game_data->keys_down['Q'])
    {
        build_terrain_from_heightmap(&terrain_mesh, &heightmap_raster); scaleY-=0.05;
    }
    if(game_data->keys_down['E'])
    {
        build_terrain_from_heightmap(&terrain_mesh, &heightmap_raster); scaleY+=0.05;
    }
    if(game_data->keys_pressed['I'])
    {
        res_const =7;
        build_terrain_from_heightmap(&terrain_mesh, &heightmap_raster);
    }
    if(game_data->keys_pressed['O'])
    {
        res_const =1;
        build_terrain_from_heightmap(&terrain_mesh, &heightmap_raster);
    }
    if(game_data->keys_pressed['P'])
    {
        drawFullTerrain ^=1;
    }
    if(game_data->keys_pressed['R'])
    {
        renderLines ^= 1;
    }


    if(game_data->keys_down[RAFGL_KEY_LEFT_CONTROL]) move_speed = 5;
    else
        move_speed = 5;


    if(game_data->keys_down[RAFGL_KEY_ESCAPE]) glfwSetWindowShouldClose(window, GLFW_TRUE);



    if(game_data->keys_down[RAFGL_KEY_SPACE]) camera_position.y += move_speed * delta_time;
    if(game_data->keys_down[RAFGL_KEY_LEFT_SHIFT]) camera_position.y -= move_speed * delta_time;


    float aspect = ((float)(game_data->raster_width)) / game_data->raster_height;
    projection = m4_perspective(fov, aspect, 0.1f, 100.0f);

    if(!game_data->keys_down['T'])
    {
        view = m4_look_at(camera_position, v3_add(camera_position, aim_dir), camera_up);
    }
    else
    {
        view = m4_look_at(camera_position, vec3(0.0f, 0.0f, 0.0f), camera_up);
    }

    model = m4_identity();

    view_projection = m4_mul(projection, view);


}

float brb = 0;
void main_state_render(GLFWwindow *window, void *args)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(shader_program_id);

    glUniformMatrix4fv(uni_M, 1, GL_FALSE, (void*)model.m);
    glUniformMatrix4fv(uni_VP, 1, GL_FALSE, (void*)view_projection.m);

    /*glUniform3f(uni_object_colour, object_colour.x, object_colour.y, object_colour.z);
    glUniform3f(uni_light_colour, light_colour.x, light_colour.y, light_colour.z);
    glUniform3f(uni_light_direction, light_direction.x, light_direction.y, light_direction.z);
    glUniform3f(uni_ambient, ambient.x, ambient.y, ambient.z);
*/
    // Crtanje terena
    /*glBindVertexArray(terrain_vao);
glDrawArrays(GL_LINES, 0, line_vertex_count);
glBindVertexArray(0);*/

glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rock_tex.tex_id);

    glUniform1i(glGetUniformLocation(shader_program_id, "rock"), 0);

glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, lava_tex.tex_id);

    glUniform1i(glGetUniformLocation(shader_program_id, "lava"), 1);


glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, lava_normal_tex.tex_id);

    glUniform1i(glGetUniformLocation(shader_program_id, "lavaNormalMap"), 2);

glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, rock_normal_tex.tex_id);

    glUniform1i(glGetUniformLocation(shader_program_id, "rockNormalMap"), 3);





    glUniform1f(glGetUniformLocation(shader_program_id, "min_height"), 0.0f);
    glUniform1f(glGetUniformLocation(shader_program_id, "max_height"), 2.0f);



    if(drawFullTerrain == 1)
    {
        if(renderLines == 0)
        {
            glBindVertexArray(terrain_vao);
            GLint sc = glGetUniformLocation(shader_program_id, "scale");
            glUniform1f(sc, 100);
            glDrawElements(GL_TRIANGLES, total_indices, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
            glUniform1f(sc, 0);
        }
        else
        {
            glBindVertexArray(terrain_vao);
            GLint sc = glGetUniformLocation(shader_program_id, "scale");
            glUniform1f(sc, 100);
            glDrawElements(GL_LINES, total_indices, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
            glUniform1f(sc, 0);
        }
    }
    else
    {
        if(res_const == 1)
        {
            for(int i = 0; i < 4; i++)
            {

                glBindVertexArray(terrain_mesh_part[i].vao);
                if(renderLines == 0)
                {


                    if(rafgl_distance2D(camera_position.x,camera_position.z,
                                       terrain_mesh_part_low[i].center.x,
                                       terrain_mesh_part_low[i].center.z) >10 )//v3_distance(camera_position, mesh->center);
                   {

                       glBindVertexArray(0);
                       glBindVertexArray(terrain_mesh_part_low[i].vao);
                       glDrawElements(GL_TRIANGLES, terrain_mesh_part_low[i].index_count, GL_UNSIGNED_INT, 0);

                   }
                    else
                    {
                        glBindVertexArray(0);
                        glBindVertexArray(terrain_mesh_part[i].vao);
                        glDrawElements(GL_TRIANGLES, terrain_mesh_part[i].index_count, GL_UNSIGNED_INT, 0);
                    }
                }
                else
                {
                    if(rafgl_distance2D(camera_position.x,camera_position.z,
                                       terrain_mesh_part_low[i].center.x,
                                       terrain_mesh_part_low[i].center.z) >10 )//v3_distance(camera_position, mesh->center);
                   {

                       glBindVertexArray(0);
                       glBindVertexArray(terrain_mesh_part_low[i].vao);
                       glDrawElements(GL_LINES, terrain_mesh_part_low[i].index_count, GL_UNSIGNED_INT, 0);

                   }
                    else
                    {
                        glBindVertexArray(0);
                        glBindVertexArray(terrain_mesh_part[i].vao);
                        glDrawElements(GL_LINES, terrain_mesh_part[i].index_count, GL_UNSIGNED_INT, 0);
                    }
                }
                glBindVertexArray(0);
            }
        }
        else
        {
            //crta sve chunnkove
            for(int i = 0; i < 4; i++)
            {
                glBindVertexArray(terrain_mesh_part_low[i].vao);
                if(renderLines == 0)
                {
                    glDrawElements(GL_TRIANGLES, terrain_mesh_part_low[i].index_count, GL_UNSIGNED_INT, 0);
                }
                else
                {
                    glDrawElements(GL_LINES, terrain_mesh_part_low[i].index_count, GL_UNSIGNED_INT, 0);
                }
                glBindVertexArray(0);
            }
        }

    }



    glBindVertexArray(0);

    // Crtanje majmuna
    /*glBindVertexArray(monkey.vao_id);
    glDrawArrays(GL_TRIANGLES, 0, monkey.vertex_count);
    glBindVertexArray(0);*/
}



void main_state_cleanup(GLFWwindow *window, void *args)
{
    glDeleteBuffers(1, &terrain_vbo);
    glDeleteVertexArrays(1, &terrain_vao);
    glDeleteShader(shader_program_id);
}

