uniform float dt;

void main()
{

    // transform the vertex position
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    float angulo = 0.0; //-3.1416/6.0;//(45.0*3.1416)/180.0; 
    float ex =1.0;
    float ey = 1.0;//  - 0.86602;
    float sx = 1.0;//3.1416/6.0;
    float sy = 1.0;
    mat4 transformacion =
        mat4(ex*cos(angulo),sx*-sin(angulo),0.0,0.0,
             sy*sin(angulo),ey*cos(angulo),0.0,0.0,
             0.0,0.0,1.0,0.0,
             0.0,0.0,0.0,1.0);

    gl_Position *= transformacion;

    // transform the texture coordinates
    gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

    // forward the vertex color
    gl_FrontColor = gl_Color;
}            
