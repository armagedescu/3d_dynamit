//! Geometry utilities - matrices, vectors, transforms.
//!
//! Port of C++ geometry.h


/// 4x4 matrix (column-major, like OpenGL/GLM)
pub type Mat4 = [f32; 16];

/// 3x3 matrix (column-major)
pub type Mat3 = [f32; 9];

/// 3D vector
pub type Vec3 = [f32; 3];

/// 4D vector
pub type Vec4 = [f32; 4];

/// Create an identity 4x4 matrix
pub fn identity_mat4() -> Mat4 {
    [
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
    ]
}

/// Create a zero 4x4 matrix
pub fn null_mat4() -> Mat4 {
    [0.0; 16]
}

/// Create an identity 3x3 matrix
pub fn identity_mat3() -> Mat3 {
    [
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0,
    ]
}

/// Create a translation matrix
pub fn translation_mat4(tx: f32, ty: f32, tz: f32) -> Mat4 {
    [
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        tx,  ty,  tz,  1.0,
    ]
}

/// Create a scale matrix
pub fn scale_mat4(sx: f32, sy: f32, sz: f32) -> Mat4 {
    [
        sx,  0.0, 0.0, 0.0,
        0.0, sy,  0.0, 0.0,
        0.0, 0.0, sz,  0.0,
        0.0, 0.0, 0.0, 1.0,
    ]
}

/// Create a rotation matrix around X axis
pub fn rotation_x_mat4(angle: f32) -> Mat4 {
    let c = angle.cos();
    let s = angle.sin();
    [
        1.0, 0.0, 0.0, 0.0,
        0.0, c,   s,   0.0,
        0.0, -s,  c,   0.0,
        0.0, 0.0, 0.0, 1.0,
    ]
}

/// Create a rotation matrix around Y axis
pub fn rotation_y_mat4(angle: f32) -> Mat4 {
    let c = angle.cos();
    let s = angle.sin();
    [
        c,   0.0, -s,  0.0,
        0.0, 1.0, 0.0, 0.0,
        s,   0.0, c,   0.0,
        0.0, 0.0, 0.0, 1.0,
    ]
}

/// Create a rotation matrix around Z axis
pub fn rotation_z_mat4(angle: f32) -> Mat4 {
    let c = angle.cos();
    let s = angle.sin();
    [
        c,   s,   0.0, 0.0,
        -s,  c,   0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
    ]
}

/// Multiply two 4x4 matrices (a * b)
pub fn multiply_mat4(a: &Mat4, b: &Mat4) -> Mat4 {
    let mut result = [0.0f32; 16];
    for row in 0..4 {
        for col in 0..4 {
            result[row + col * 4] =
                a[row + 0 * 4] * b[0 + col * 4] +
                a[row + 1 * 4] * b[1 + col * 4] +
                a[row + 2 * 4] * b[2 + col * 4] +
                a[row + 3 * 4] * b[3 + col * 4];
        }
    }
    result
}

/// Transform a position by a matrix
pub fn transform_position(m: &Mat4, x: f32, y: f32, z: f32) -> Vec3 {
    let w = 1.0;
    [
        m[0] * x + m[4] * y + m[8] * z + m[12] * w,
        m[1] * x + m[5] * y + m[9] * z + m[13] * w,
        m[2] * x + m[6] * y + m[10] * z + m[14] * w,
    ]
}

/// Transform a normal by a matrix (applies inverse transpose for correct normal transformation)
pub fn transform_normal(m: &Mat4, nx: f32, ny: f32, nz: f32) -> Vec3 {
    // For correct normal transformation under non-uniform scale,
    // we need to apply the inverse transpose of the 3x3 upper-left
    let col0_sq = m[0] * m[0] + m[1] * m[1] + m[2] * m[2];
    let col1_sq = m[4] * m[4] + m[5] * m[5] + m[6] * m[6];
    let col2_sq = m[8] * m[8] + m[9] * m[9] + m[10] * m[10];

    let r00 = m[0] / col0_sq;
    let r10 = m[1] / col0_sq;
    let r20 = m[2] / col0_sq;
    let r01 = m[4] / col1_sq;
    let r11 = m[5] / col1_sq;
    let r21 = m[6] / col1_sq;
    let r02 = m[8] / col2_sq;
    let r12 = m[9] / col2_sq;
    let r22 = m[10] / col2_sq;

    let tnx = r00 * nx + r01 * ny + r02 * nz;
    let tny = r10 * nx + r11 * ny + r12 * nz;
    let tnz = r20 * nx + r21 * ny + r22 * nz;

    let len = (tnx * tnx + tny * tny + tnz * tnz).sqrt();
    if len > 0.0001 {
        [tnx / len, tny / len, tnz / len]
    } else {
        [tnx, tny, tnz]
    }
}

/// Normalize a vector
pub fn normalize(v: Vec3) -> Vec3 {
    let len = (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]).sqrt();
    if len > 0.0001 {
        [v[0] / len, v[1] / len, v[2] / len]
    } else {
        v
    }
}

/// Cross product of two vectors
pub fn cross(a: Vec3, b: Vec3) -> Vec3 {
    [
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0],
    ]
}

/// Dot product of two vectors
pub fn dot(a: Vec3, b: Vec3) -> f32 {
    a[0] * b[0] + a[1] * b[1] + a[2] * b[2]
}

/// Calculate normal from three points (left-handed cross product)
pub fn cross_3points(p1: Vec3, p2: Vec3, p3: Vec3) -> Vec3 {
    let v1 = [p2[0] - p1[0], p2[1] - p1[1], p2[2] - p1[2]];
    let v2 = [p3[0] - p1[0], p3[1] - p1[1], p3[2] - p1[2]];

    // Left-handed cross product
    [
        v1[2] * v2[1] - v1[1] * v2[2],
        v1[0] * v2[2] - v1[2] * v2[0],
        v1[1] * v2[0] - v1[0] * v2[1],
    ]
}

/// Create a perspective projection matrix
pub fn perspective(fov_y: f32, aspect: f32, near: f32, far: f32) -> Mat4 {
    let f = 1.0 / (fov_y / 2.0).tan();
    let nf = 1.0 / (near - far);

    [
        f / aspect, 0.0, 0.0, 0.0,
        0.0, f, 0.0, 0.0,
        0.0, 0.0, (far + near) * nf, -1.0,
        0.0, 0.0, 2.0 * far * near * nf, 0.0,
    ]
}

/// Create a look-at view matrix
pub fn look_at(eye: Vec3, center: Vec3, up: Vec3) -> Mat4 {
    let f = normalize([
        center[0] - eye[0],
        center[1] - eye[1],
        center[2] - eye[2],
    ]);
    let s = normalize(cross(f, up));
    let u = cross(s, f);

    [
        s[0], u[0], -f[0], 0.0,
        s[1], u[1], -f[1], 0.0,
        s[2], u[2], -f[2], 0.0,
        -dot(s, eye), -dot(u, eye), dot(f, eye), 1.0,
    ]
}

/// Apply a transform to a range of vertices and normals
pub fn apply_transform_to_range(
    m: &Mat4,
    verts: &mut [f32],
    norms: &mut [f32],
    start_vertex: usize,
) {
    let start_idx = start_vertex * 3;

    // Transform vertices
    let mut i = start_idx;
    while i + 2 < verts.len() {
        let [nx, ny, nz] = transform_position(m, verts[i], verts[i + 1], verts[i + 2]);
        verts[i] = nx;
        verts[i + 1] = ny;
        verts[i + 2] = nz;
        i += 3;
    }

    // Transform normals
    let mut i = start_idx;
    while i + 2 < norms.len() {
        let [nx, ny, nz] = transform_normal(m, norms[i], norms[i + 1], norms[i + 2]);
        norms[i] = nx;
        norms[i + 1] = ny;
        norms[i + 2] = nz;
        i += 3;
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_identity() {
        let m = identity_mat4();
        assert_eq!(m[0], 1.0);
        assert_eq!(m[5], 1.0);
        assert_eq!(m[10], 1.0);
        assert_eq!(m[15], 1.0);
    }

    #[test]
    fn test_transform_position() {
        let m = translation_mat4(1.0, 2.0, 3.0);
        let [x, y, z] = transform_position(&m, 0.0, 0.0, 0.0);
        assert!((x - 1.0).abs() < 1e-6);
        assert!((y - 2.0).abs() < 1e-6);
        assert!((z - 3.0).abs() < 1e-6);
    }

    #[test]
    fn test_normalize() {
        let v = normalize([3.0, 0.0, 4.0]);
        assert!((v[0] - 0.6).abs() < 1e-6);
        assert!((v[2] - 0.8).abs() < 1e-6);
    }
}
