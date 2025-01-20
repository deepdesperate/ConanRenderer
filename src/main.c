#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "array.h"
#include "display.h"
#include "vector.h"
#include "matrix.h"
#include "mesh.h"

triangle_t* triangles_to_render = NULL;

vec3_t camera_position = { 0, 0, 0 };
vec3_t cube_rotation = { .x = 0, .y = 0, .z = 0};

float fov_factor = 640;

bool is_running = false;
int previous_frame_time = 0;

void setup(void) {
    // Initialize render mode and triangle culling method
    render_method = RENDER_WIRE;
    cull_method = CULL_BACKFACE;

    // Allocate the required memory in bytes to hold the color buffer
    color_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_width * window_height);

    // Creating a SDL texture that is used to display the color buffer
    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height
    );

    // Loads the cub values in our mesh data structure
    load_cube_mesh_data();
    // load_obj_file_data("./assets/cube.obj");
    // load_obj_file_data("./assets/f22.obj");

}

void process_input(void) {
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type) {
        case SDL_QUIT:
            is_running = false;
            break;
        case SDL_KEYDOWN:
            if( event.key.keysym.sym == SDLK_ESCAPE)
                is_running = false;
            if (event.key.keysym.sym == SDLK_1)
                render_method = RENDER_WIRE_VERTEX;
            if (event.key.keysym.sym == SDLK_2)
                render_method = RENDER_WIRE;
            if (event.key.keysym.sym == SDLK_3)
                render_method = RENDER_FILL_TRIANGLE;
            if (event.key.keysym.sym == SDLK_4)
                render_method = RENDER_FILL_TRIANGLE_WIRE;
            if (event.key.keysym.sym == SDLK_c)
                cull_method = CULL_BACKFACE;
            if (event.key.keysym.sym == SDLK_d)
                cull_method = CULL_NONE;
            break;
    }
}

// Function that recieves a 3D vector and returns a projected 2D point

vec2_t project(vec3_t point){
    vec2_t projected_point = {
        .x = (fov_factor * point.x) / point.z,
        .y = (fov_factor * point.y) / point.z
    };
    return projected_point;
}

void update(void){
    // while (SDL_TICKS_PASSED(SDL_GetTicks(), previous_frame_time + FRAME_TARGET_TIME))

    previous_frame_time = SDL_GetTicks();

    // Initialize the array of triangles to render
    triangles_to_render = NULL;

    mesh.rotation.x += 0.001;
    mesh.rotation.y += 0.001;
    mesh.rotation.z += 0.001;

    // mesh.scale.x += 0.002;
    // mesh.scale.y += 0.001;
    mesh.translation.x += 0.001;
    mesh.translation.z = 5.0;


    // Create a scale matrix that will be used to multiply the mesh vertices
    mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
    mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
    mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
    mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);
    mat4_t translation_matrix = mat4_make_translation(mesh.translation.x , mesh.translation.y, mesh.translation.z);

    // Loop all triangle faces of our mesh
    int num_faces = array_length(mesh.faces);
    for (int i = 0; i < num_faces; i++){
        face_t mesh_face = mesh.faces[i];
        
        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertices[mesh_face.a - 1];
        face_vertices[1] = mesh.vertices[mesh_face.b - 1];
        face_vertices[2] = mesh.vertices[mesh_face.c - 1];

        vec4_t transformed_vertices[3];

        // loop  all three vertices of the current face and apply transformations
        for( int j = 0; j < 3; j++ ){
            vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

            // Use a matrix to scale our original vertex
            transformed_vertex = mat4_mul_vec4(scale_matrix, transformed_vertex);
            transformed_vertex = mat4_mul_vec4(rotation_matrix_x, transformed_vertex);
            transformed_vertex = mat4_mul_vec4(rotation_matrix_y, transformed_vertex);
            transformed_vertex = mat4_mul_vec4(rotation_matrix_z, transformed_vertex);
            transformed_vertex = mat4_mul_vec4(translation_matrix, transformed_vertex);

            // Save transformed vertex in the array of transformed vertices
            transformed_vertices[j] = transformed_vertex;
        }

        // Backface culling condition
        if (cull_method == CULL_BACKFACE){
            // Check backfaces culling
            vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]);  /*   A   */
            vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]);  /*  / \  */
            vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]);  /* C---B */

            vec3_t vector_ab = vec3_sub(vector_b, vector_a);
            vec3_t vector_ac = vec3_sub(vector_c, vector_a);
            vec3_normalize(&vector_ab);
            vec3_normalize(&vector_ac);

            // Compute the face normal (using cross product to find perpendicular )
            vec3_t normal = vec3_cross(vector_ab, vector_ac);
            vec3_normalize(&normal);

            // Find the vector between a point in the triangle and the camera origin
            vec3_t camera_ray = vec3_sub(camera_position, vector_a);

            // Calculate how aligned the camera ray is with the face normal (using dot product)
            float dot_normal_camera = vec3_dot(normal, camera_ray);

            // Bypass the triangle that are looking away from the camera
            if ( dot_normal_camera < 0 ) {
                continue;
            }
        }


        vec2_t projected_ponts[3];
        // Loop all three vertices to perform projection
        for (int j = 0; j < 3; j++){
            // Project the current vertex
            projected_ponts[j] = project(vec3_from_vec4(transformed_vertices[j]));

            // Scale and translate the projected point to the center of the screen
            projected_ponts[j].x += (window_width / 2);
            projected_ponts[j].y += (window_height / 2);
        }

        // Calculate the average depth for each face based on the vertices after transformation
        float avg_depth = (transformed_vertices[0].z + transformed_vertices[1].z +transformed_vertices[2].z) / 3.0;
        
        triangle_t projected_triangle = {
            .points = {
                {projected_ponts[0].x, projected_ponts[0].y},
                {projected_ponts[1].x, projected_ponts[1].y},
                {projected_ponts[2].x, projected_ponts[2].y},
            },
            .color = mesh_face.color,
            .avg_depth = avg_depth
            // TODO: Avg depth per triangle
        };

        // Save the projected triangle in the array of triangles to render
        // triangles_to_render[i] = projected_triangle;
        array_push(triangles_to_render, projected_triangle);
    }

    // Sort the triangles to render by their avg_depth using Bubble Sort
    int num_triangles = array_length(triangles_to_render);
    for(int i = 0; i < num_triangles; i++ ){
        for(int j = i; j < num_triangles; j++){
            if(triangles_to_render[i].avg_depth < triangles_to_render[j].avg_depth){
                // Swap the triangles positions in the array
                triangle_t temp = triangles_to_render[i];
                triangles_to_render[i] = triangles_to_render[j];
                triangles_to_render[j] = temp;
            }
        }
    }

}

void render(void){
    draw_grid();

    // Loop all projected triangles and render them
    int num_triangles = array_length(triangles_to_render);
    
    for( int i = 0; i < num_triangles; i++ ) {
        triangle_t triangle = triangles_to_render[i];

        // Draw filled triangles
        if( render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE){
            draw_filled_triangle(
                triangle.points[0].x, triangle.points[0].y,
                triangle.points[1].x, triangle.points[1].y,
                triangle.points[2].x, triangle.points[2].y,
                triangle.color
            );
        }

        // Draw unfilled triangles -- Wireframe
        if (render_method == RENDER_WIRE || render_method == RENDER_WIRE_VERTEX || render_method == RENDER_FILL_TRIANGLE_WIRE){
            draw_triangle(
                triangle.points[0].x, triangle.points[0].y,
                triangle.points[1].x, triangle.points[1].y,
                triangle.points[2].x, triangle.points[2].y,
                0xFFFFFFFF
            );
        }

        // // Draw vertex points
        if( render_method == RENDER_WIRE_VERTEX){
            draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFFFFFF00);
            draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFFFFFF00);
            draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFFFFFF00);
        }
    }

    // Clear the array of triangles to render every frame loop
    array_free(triangles_to_render);

    render_color_buffer();
    clear_color_buffer(0xFF000000);

    SDL_RenderPresent(renderer);
}

// Free memory that was dynamically allocated by the program
void free_resources(void){
    free(color_buffer);
    array_free(mesh.faces);
    array_free(mesh.vertices);
}

int main(void){
    is_running = initialize_window();

    setup();

    while(is_running){
        process_input();
        update();
        render();
    }

    destroy_window();
    free_resources();
    
    return 0;
}