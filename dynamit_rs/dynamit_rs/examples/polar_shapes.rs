//! Example: Generate polar shapes using PolarBuilder
//!
//! This example demonstrates generating cone and cylinder geometry
//! with various formula-based variations.

use dynamit_rs::PolarBuilder;
use std::f32::consts::PI;

fn main() {
    println!("=== Dynamit RS - PolarBuilder Demo ===\n");

    // Example 1: Basic cone
    println!("1. Basic Cone (radius=1, sectors=8)");
    let mut verts = Vec::new();
    let mut norms = Vec::new();
    let mut indices = Vec::new();

    PolarBuilder::new()
        .formula("1")
        .sectors(8)
        .build_cone_indexed(&mut verts, &mut norms, &mut indices)
        .unwrap();

    println!("   Vertices: {} ({} triangles)", verts.len() / 3, indices.len() / 3);
    println!("   First vertex: ({:.2}, {:.2}, {:.2})", verts[0], verts[1], verts[2]);

    // Example 2: Sinusoidal cone
    println!("\n2. Sinusoidal Cone (formula: 1 + 0.3*sin(4*theta))");
    verts.clear();
    norms.clear();
    indices.clear();

    PolarBuilder::new()
        .formula("1 + 0.3*sin(4*theta)")
        .sectors(32)
        .build_cone_indexed(&mut verts, &mut norms, &mut indices)
        .unwrap();

    println!("   Vertices: {} ({} triangles)", verts.len() / 3, indices.len() / 3);

    // Example 3: Basic cylinder
    println!("\n3. Basic Cylinder (radius=1, slices=4)");
    verts.clear();
    norms.clear();
    indices.clear();

    PolarBuilder::new()
        .formula("1")
        .sectors(12)
        .slices(4)
        .build_cylinder_indexed(&mut verts, &mut norms, &mut indices)
        .unwrap();

    println!("   Vertices: {} ({} triangles)", verts.len() / 3, indices.len() / 3);

    // Example 4: Tapered cylinder (cone-like via formula)
    println!("\n4. Varying Radius Cylinder (formula: 0.5 + 0.5*y)");
    verts.clear();
    norms.clear();
    indices.clear();

    PolarBuilder::new()
        .formula("0.5 + 0.5*y")
        .sectors(16)
        .slices(8)
        .build_cylinder_indexed(&mut verts, &mut norms, &mut indices)
        .unwrap();

    println!("   Vertices: {} ({} triangles)", verts.len() / 3, indices.len() / 3);

    // Example 5: Star-shaped double-coated cone
    println!("\n5. Star Cone (double-coated, formula: 0.8 + 0.2*cos(5*theta))");
    let mut colors = Vec::new();
    verts.clear();
    norms.clear();
    indices.clear();

    PolarBuilder::new()
        .formula("0.8 + 0.2*cos(5*theta)")
        .sectors(40)
        .double_coated(true)
        .color_outer(1.0, 0.8, 0.0, 1.0)  // Gold outside
        .color_inner(0.2, 0.0, 0.4, 1.0)  // Purple inside
        .build_cone_indexed_with_color(&mut verts, &mut norms, &mut colors, &mut indices)
        .unwrap();

    println!("   Vertices: {} ({} triangles)", verts.len() / 3, indices.len() / 3);
    println!("   Has colors: {} floats ({} vertices * 4 RGBA)", colors.len(), colors.len() / 4);

    // Example 6: Half-domain cone (semicircle)
    println!("\n6. Half Cone (domain: 0 to PI)");
    verts.clear();
    norms.clear();
    indices.clear();

    PolarBuilder::new()
        .formula("1")
        .domain(0.0, PI)
        .sectors(12)
        .build_cone_indexed(&mut verts, &mut norms, &mut indices)
        .unwrap();

    println!("   Vertices: {} ({} triangles)", verts.len() / 3, indices.len() / 3);

    println!("\n=== Demo Complete ===");
}
