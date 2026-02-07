//! Dynamit Demo - Automatic shader generation
//!
//! This example demonstrates using Dynamit with automatic shader generation.
//! No manual shader code required!

use std::ffi::CString;
use std::num::NonZeroU32;
use std::f32::consts::PI;
use std::rc::Rc;

use glow::HasContext;
use glutin::config::ConfigTemplateBuilder;
use glutin::context::{ContextApi, ContextAttributesBuilder, Version};
use glutin::display::GetGlDisplay;
use glutin::prelude::*;
use glutin::surface::SwapInterval;
use glutin_winit::{DisplayBuilder, GlWindow};
use raw_window_handle::HasRawWindowHandle;
use winit::event::{Event, KeyEvent, WindowEvent};
use winit::event_loop::EventLoop;
use winit::keyboard::{Key, NamedKey};
use winit::window::WindowBuilder;

use dynamit_rs::{Dynamit, PolarBuilder, perspective, look_at, multiply_mat4, rotation_y_mat4, rotation_x_mat4};

fn main() {
    let event_loop = EventLoop::new().unwrap();

    let window_builder = WindowBuilder::new()
        .with_title("Dynamit RS - Auto Shader Demo")
        .with_inner_size(winit::dpi::LogicalSize::new(800, 600));

    let template = ConfigTemplateBuilder::new().with_alpha_size(8);
    let display_builder = DisplayBuilder::new().with_window_builder(Some(window_builder));

    let (window, gl_config) = display_builder
        .build(&event_loop, template, |configs| {
            configs.reduce(|accum, config| {
                if config.num_samples() > accum.num_samples() { config } else { accum }
            }).unwrap()
        })
        .unwrap();

    let window = window.unwrap();
    let raw_window_handle = window.raw_window_handle();
    let gl_display = gl_config.display();

    let context_attributes = ContextAttributesBuilder::new()
        .with_context_api(ContextApi::OpenGl(Some(Version::new(3, 3))))
        .build(Some(raw_window_handle));

    let gl_context = unsafe {
        gl_display.create_context(&gl_config, &context_attributes).unwrap()
    };

    let attrs = window.build_surface_attributes(<_>::default());
    let gl_surface = unsafe {
        gl_config.display().create_window_surface(&gl_config, &attrs).unwrap()
    };

    let gl_context = gl_context.make_current(&gl_surface).unwrap();

    let gl = Rc::new(unsafe {
        glow::Context::from_loader_function(|s| {
            let s = CString::new(s).unwrap();
            gl_display.get_proc_address(&s) as *const _
        })
    });

    gl_surface
        .set_swap_interval(&gl_context, SwapInterval::Wait(NonZeroU32::new(1).unwrap()))
        .ok();

    // ========================================
    // Generate geometry using PolarBuilder
    // ========================================
    let mut verts = Vec::new();
    let mut norms = Vec::new();
    let mut colors = Vec::new();
    let mut indices = Vec::new();

    PolarBuilder::new()
        .formula("1 + 0.3*sin(5*theta)")
        .sectors(48)
        .double_coated(true)
        .color_outer(1.0, 0.6, 0.1, 1.0)  // Orange
        .color_inner(0.2, 0.4, 0.8, 1.0)  // Blue
        .build_cone_indexed_with_color(&mut verts, &mut norms, &mut colors, &mut indices)
        .unwrap();

    println!("Generated {} vertices, {} indices", verts.len() / 3, indices.len());

    // ========================================
    // Create Dynamit shape - SHADERS AUTO-GENERATED!
    // ========================================
    let mut shape = Dynamit::new(gl.clone())
        .with_vertices_3d(&verts)
        .with_normals_3d(&norms)
        .with_colors_4d(&colors)
        .with_indices(&indices)
        .with_const_light_direction(0.5, 0.5, 1.0)
        .with_transform_matrix_4f("transformMatrix");

    // Log the auto-generated shaders
    println!("\n--- Auto-generated shaders ---");
    shape.log_generated_shaders();
    println!("--- End shaders ---\n");

    // Build the program
    shape.build_program().expect("Failed to build program");

    unsafe {
        gl.enable(glow::DEPTH_TEST);
        gl.clear_color(0.1, 0.1, 0.15, 1.0);
    }

    let mut angle = 0.0f32;

    event_loop
        .run(move |event, elwt| {
            match event {
                Event::WindowEvent { event, .. } => match event {
                    WindowEvent::CloseRequested => elwt.exit(),
                    WindowEvent::KeyboardInput {
                        event: KeyEvent { logical_key: Key::Named(NamedKey::Escape), .. },
                        ..
                    } => elwt.exit(),
                    WindowEvent::Resized(size) => {
                        if size.width != 0 && size.height != 0 {
                            gl_surface.resize(
                                &gl_context,
                                NonZeroU32::new(size.width).unwrap(),
                                NonZeroU32::new(size.height).unwrap(),
                            );
                            unsafe { gl.viewport(0, 0, size.width as i32, size.height as i32); }
                        }
                    }
                    WindowEvent::RedrawRequested => {
                        angle += 0.01;

                        let size = window.inner_size();
                        let aspect = size.width as f32 / size.height as f32;

                        // Build MVP matrix using dynamit geometry utilities
                        let projection = perspective(45.0 * PI / 180.0, aspect, 0.1, 100.0);
                        let view = look_at([0.0, 1.5, 3.0], [0.0, 0.3, 0.0], [0.0, 1.0, 0.0]);
                        let model = multiply_mat4(&rotation_y_mat4(angle), &rotation_x_mat4(-0.3));
                        let mvp = multiply_mat4(&projection, &multiply_mat4(&view, &model));

                        unsafe {
                            gl.clear(glow::COLOR_BUFFER_BIT | glow::DEPTH_BUFFER_BIT);
                        }

                        // Update uniform and draw - that's it!
                        shape.transform_matrix_4f(&mvp).expect("Failed to set transform");
                        shape.draw_triangles_indexed().expect("Failed to draw");

                        gl_surface.swap_buffers(&gl_context).unwrap();
                        window.request_redraw();
                    }
                    _ => {}
                },
                _ => {}
            }
        })
        .unwrap();
}
