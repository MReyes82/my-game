#version 110
uniform sampler2D nubes;
uniform sampler2D personaje;
uniform float dt;
uniform float delay;
void main()
{
    vec4 nube = texture2D(nubes, gl_TexCoord[0].xy);
    vec4 per = texture2D(personaje,gl_TexCoord[0].xy);
    
    float tiempo = dt*delay;
    float alpha = smoothstep(tiempo-0.05,tiempo+0.05,nube.r);
    vec4 mezcla = per;
    mezcla.a*=alpha;
    gl_FragColor =  mezcla;
}
