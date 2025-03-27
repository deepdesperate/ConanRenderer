#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "upng.h"
#include "array.h"
#include "display.h"
#include "vector.h"
#include "matrix.h"
#include "light.h"
#include "camera.h"
#include "clipping.h"
#include "mesh.h"
#include "texture.h"
#include "triangle.h"

#define MAX_TRIANGLES_PER_MESH 10000
triangle_t triangles_to_render[MAX_TRIANGLES_PER_MESH];
int num_triangles_to_render = 0;

vec3_t cube_rotation = { .x = 0, .y = 0, .z = 0};

bool is_running = false;
int previous_frame_time = 0;
float delta_time = 0;

mat4_t world_matrix;
mat4_t proj_matrix;
mat4_t view_matrix;

void setup(void) {
    // Initialize render mode and triangle culling method
    // render_method = RENDER_TEXTURED_WIRE;
    // cull_method = CULL_BACKFACE;

    set_render_method(RENDER_WIRE);
    set_cull_method(CULL_BACKFACE);

    // Initializa the scene light direction
    init_light(vec3_new(0, 0, 1));

    // Initialize the perspective projection matrix
    float aspectx = (float)get_window_width() / (float)get_window_height() ;
    float aspecty = (float)get_window_height() / (float)get_window_width();
    float fovy = M_PI / 3.0;
    float fovx = atan(tan(fovy / 2) * aspectx) * 2.0;
    float z_near = 1.0;
    float z_far = 20.0;
    proj_matrix = mat4_make_perspective(fovy, aspecty, z_near, z_far);

    // Initialize frustum planes with a point and a normal
    init_frustum_planes(fovx, fovy, z_near, z_far);

    // TODO: obj, tex, scale, translation, rot
    load_mesh("./assets/f22.obj", "./assets/f22.png", vec3_new(1, 1, 1), vec3_new(-3, 0, +3), vec3_new(0, 0, 0));
    load_mesh("./assets/efa.obj", "./assets/efa.png", vec3_new(1, 1, 1), vec3_new(+3, 0, +3), vec3_new(0, 0, 0));
    
}

void process_input(void) {
    SDL_Event event;
    while(SDL_PollEvent(&event)){
        switch (event.type) {
            case SDL_QUIT:
                is_running = false;
                break;
            case SDL_KEYDOWN:
                if( event.key.keysym.sym == SDLK_ESCAPE){
                    is_running = false;
                    break;
                }
                if (event.key.keysym.sym == SDLK_1){
                    set_render_method(RENDER_WIRE_VERTEX);
                    break;
                }
                if (event.key.keysym.sym == SDLK_2){
                    set_render_method(RENDER_WIRE);
                    break;
                }
                if (event.key.keysym.sym == SDLK_3){
                    set_render_method(RENDER_FILL_TRIANGLE);
                    break;
                }
                if (event.key.keysym.sym == SDLK_4){
                    set_render_method(RENDER_FILL_TRIANGLE_WIRE);
                    break;
                }
                if (event.key.keysym.sym == SDLK_5){
                    set_render_method(RENDER_TEXTURED);
                    break;
                }
                if (event.key.keysym.sym == SDLK_6){
                    set_render_method(RENDER_TEXTURED_WIRE);
                    break;
                }
                if (event.key.keysym.sym == SDLK_c){
                    set_cull_method(CULL_BACKFACE);
                    break;
                }
                if (event.key.keysym.sym == SDLK_x){
                    set_cull_method(CULL_NONE);
                    break;
                }
                if (event.key.keysym.sym == SDLK_UP){
                    update_camera_forward_velocity(vec3_mul(get_camera_direction(), 5.0 * delta_time));
                    update_camera_position(vec3_add(get_camera_direction(), get_camera_forward_velocity()));
                    break;
                }
                if (event.key.keysym.sym == SDLK_DOWN){
                    update_camera_forward_velocity(vec3_mul(get_camera_direction(), 5.0 * delta_time));
                    update_camera_position(vec3_sub(get_camera_direction(), get_camera_forward_velocity()));
                    break;
                }
                if (event.key.keysym.sym == SDLK_w){
                    rotate_camera_pitch(+3.0 * delta_time);
                    break;
                }
                if (event.key.keysym.sym == SDLK_s){
                    rotate_camera_pitch(-3.0 * delta_time);
                    break;
                }
                if(event.key.keysym.sym == SDLK_RIGHT){
                    rotate_camera_yaw(+1.0 * delta_time);
                    break;
                }
                if(event.key.keysym.sym == SDLK_LEFT){
                    rotate_camera_yaw(-1.0 * delta_time);
                    break;
                }
                break;
        }
    }
}


void update(void){
    // while (SDL_TICKS_PASSED(SDL_GetTicks(), previous_frame_time + FRAME_TARGET_TIME))

    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }

    // Get a delta time factor converted to bseconds to be used to update our game object
    delta_time = (SDL_GetTicks() - previous_frame_time) / 1000.0 ;

    previous_frame_time = SDL_GetTicks();

    // Initialize the counter of triangles to render for the current fram
    num_triangles_to_render = 0;

    // Loop all the meshes of our scene
    for (int mesh_index = 0; mesh_index < get_num_meshes(); mesh_index++){
        mesh_t* mesh = get_mesh(mesh_index);
        // mesh.rotation.x += 0.0 * delta_time;
        // mesh.rotation.y += 0.3 * delta_time;
        // mesh.rotation.z += 0.0 * delta_time;
        // mesh.translation.z = 4.0;

        // Create a scale matrix that will be used to multiply the mesh vertices
        mat4_t scale_matrix = mat4_make_scale(mesh->scale.x, mesh->scale.y, mesh->scale.z);
        mat4_t translation_matrix = mat4_make_translation(mesh->translation.x , mesh->translation.y, mesh->translation.z);
        mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh->rotation.x);
        mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh->rotation.y);
        mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh->rotation.z);

        // Offset the camera position in the direction where the camera is poiting at
        vec3_t target = get_camera_lookat_target();
        vec3_t up_direction = vec3_new(0, 1, 0);
        view_matrix = mat4_look_at(get_camera_position(), target, up_direction );

        // Loop all triangle faces of our mesh
        int num_faces = array_length(mesh->faces);
        for (int i = 0; i < num_faces; i++){
            face_t mesh_face = mesh->faces[i];
            
            vec3_t face_vertices[3];
            face_vertices[0] = mesh->vertices[mesh_face.a ];
            face_vertices[1] = mesh->vertices[mesh_face.b ];
            face_vertices[2] = mesh->vertices[mesh_face.c ];

            vec4_t transformed_vertices[3];

            // loop  all three vertices of the current face and apply transformations
            for( int j = 0; j < 3; j++ ){
                vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

                // Create a World Matrix combining scale, rotation and translation matrices
                // [L] * [R] * [S] * [Identity ] = [World_matrix]

                mat4_t world_matrix = mat4_identity();
                world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
                world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
                world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
                world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
                world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

                // Multiply the world matrix by the original vector
                transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);

                // Multipy the view matrix by the vector to transform the scene to camera space
                transformed_vertex = mat4_mul_vec4(view_matrix, transformed_vertex);

                // Save transformed vertex in the array of transformed vertices
                transformed_vertices[j] = transformed_vertex;
            }

            // Backface culling condition
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
            vec3_t origin = {0, 0, 0};
            vec3_t camera_ray = vec3_sub(origin, vector_a);

            // Calculate how aligned the camera ray is with the face normal (using dot product)
            float dot_normal_camera = vec3_dot(normal, camera_ray);

            // Bypass the triangle that are looking away from the camera
            if ( is_cull_backface() ){
                // Backface culling, bypassing triangles that are looking away from the camera
                if ( dot_normal_camera < 0 ) {
                    continue;
                }
            }

            // Create a polygon from the original transform polygon_from_triangle() -> in
            polygon_t polygon = polygon_from_triangle(
                vec3_from_vec4(transformed_vertices[0]),
                vec3_from_vec4(transformed_vertices[1]),
                vec3_from_vec4(transformed_vertices[2]),
                mesh_face.a_uv,
                mesh_face.b_uv,
                mesh_face.c_uv
                
            );

            // Clip the polygons and return a new polygon with potential new vertices
            clip_polygon(&polygon);

            // Break the clipped polygon apart back into individual triangles
            triangle_t triangles_after_clipping[MAX_NUM_POLY_TRIANGLES];
            int num_triangles_after_clippling = 0;

            triangles_from_polygon(&polygon, triangles_after_clipping, &num_triangles_after_clippling);

            // Loops all the assembled triangles after clipping
            for(int t = 0; t < num_triangles_after_clippling; t++ ){

                triangle_t triangle_after_clipping = triangles_after_clipping[t];

                vec4_t projected_points[3];

                // Loop all three vertices to perform projection
                for (int j = 0; j < 3; j++){
                    // Project the current vertex
                    projected_points[j] = mat4_mul_vec4_project(proj_matrix, triangle_after_clipping.points[j]);
                    
                    // Perform perspective divide
                    if (projected_points[j].w != 0) {
                        projected_points[j].x /= projected_points[j].w;
                        projected_points[j].y /= projected_points[j].w;
                        projected_points[j].z /= projected_points[j].w;

                    }

                    projected_points[j].y *= -1;

                    // Scale into the view
                    projected_points[j].x *= (get_window_width() / 2.0);
                    projected_points[j].y *= (get_window_height() / 2.0);

                    // Translate the projected point to the center of the screen
                    projected_points[j].x += (get_window_width() / 2.0);
                    projected_points[j].y += (get_window_height() / 2.0);
                    
                }
                
                // -ve Cuz of light direction
                float light_intensity_factor = -vec3_dot(normal, get_light_direction());

                // Calculate the triangle color based on how aligned is the normal and the inverse of the light ray
                uint32_t triangle_color = light_apply_intensity(mesh_face.color, light_intensity_factor);

                triangle_t triangle_to_render = {
                    .points = {
                        {projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w},
                        {projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w},
                        {projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w},
                    },
                    .texcoords = {
                        { triangle_after_clipping.texcoords[0].u, triangle_after_clipping.texcoords[0].v },
                        { triangle_after_clipping.texcoords[1].u, triangle_after_clipping.texcoords[1].v },
                        { triangle_after_clipping.texcoords[2].u, triangle_after_clipping.texcoords[2].v }
                    },
                    .color = triangle_color,
                    .texture = mesh->texture
                };

                // Save the projected triangle in the array of triangles to render
                // triangles_to_render[i] = projected_triangle;
                if (num_triangles_to_render < MAX_TRIANGLES_PER_MESH){
                    triangles_to_render[num_triangles_to_render++] = triangle_to_render;
                }
            }
        }
    }

}

void render(void){
    // Clear all the arrays to get ready for the next frame
    clear_color_buffer(0xFF000000);
    clear_z_buffer();

    draw_grid();

    // Loop all projected triangles and render them
    
    for( int i = 0; i < num_triangles_to_render; i++ ) {
        triangle_t triangle = triangles_to_render[i];

        // Draw filled triangles
        if( should_render_filled_triangles() ){
            draw_filled_triangle(
                triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w,
                triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w,
                triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w,
                triangle.color
            );
        }

        // Draw textured triangle
        if (should_render_textured_triangles()) {
            draw_textured_triangle(
                triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, triangle.texcoords[0].u, triangle.texcoords[0].v,
                triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, triangle.texcoords[1].u, triangle.texcoords[1].v,
                triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, triangle.texcoords[2].u, triangle.texcoords[2].v,
                triangle.texture
            );
        }

        // Draw unfilled triangles -- Wireframe
        if (should_render_wireframe()){
            draw_triangle(
                triangle.points[0].x, triangle.points[0].y,
                triangle.points[1].x, triangle.points[1].y,
                triangle.points[2].x, triangle.points[2].y,
                0xFFFFFFFF
            );
        }

        // // Draw vertex points
        if( should_render_wire_vertex()){
            draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFFFFFF00);
            draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFFFFFF00);
            draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFFFFFF00);
        }
    }

    render_color_buffer();

}

// Free memory that was dynamically allocated by the program
void free_resources(void){
    
    free_meshes();
    destroy_window();

}

int main(void){
    is_running = initialize_window();

    setup();

    while(is_running){
        process_input();
        update();
        render();
    }

    free_resources();
    
    return 0;
}