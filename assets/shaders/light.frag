#version 330

// Output of Fragment Shader is Frag Color [Pixel Color]
out vec4 frag_color;

// Varyings
in Varyings {
    vec4 color;
    vec2 tex_coord;
    vec3 normal;
    vec3 view;
    vec3 world_position; // Position in the World Space
} fs_in;

// Light Properties
#define DIRECTIONAL 0
#define POINT       1
#define SPOT        2

#define MAX_LIGHTS 8

struct Light {
    int type; // Type of the Light Source
    vec3 direction; // Direction of the Light Source
    vec3 position; // Position of the Light Source
    vec3 color; // Color of the Light
    vec3 attenuation; // Attenuation of the Light

    // Cone Angles
    float inner_cone_angle; // Theta_p
    float outer_cone_angle; // Theta_u
};

struct Material {
    sampler2D albedo;
    sampler2D specular;
    sampler2D roughness;
    sampler2D ambientOcclusion;
    sampler2D emission;
};
uniform Material material;
uniform float alphaThreshold;
uniform Light lights[MAX_LIGHTS];
uniform int light_count;
uniform vec3 ambient_light = vec3(0.7);


void main(){
    // Normalize the Varyings [Normal and View]
    vec3 view = normalize(fs_in.view);
    vec3 normal = normalize(fs_in.normal);
    vec3 world_position = fs_in.world_position;

    //1. Material Properties sampled from Textures
    vec3 material_diffuse = texture(material.albedo, fs_in.tex_coord).rgb; // Diffuse Color of the Material
    vec3 material_specular = texture(material.specular, fs_in.tex_coord).rgb;
    float material_roughness = texture(material.roughness, fs_in.tex_coord).r; 
    // Shininess of the Material
    float material_shininess = 2.0 / pow(clamp(material_roughness, 0.001, 0.999), 4.0) - 2.0;
    vec3 material_ambient = material_diffuse * texture(material.ambientOcclusion, fs_in.tex_coord).r; // Ambient Occlusion Map we will use only 1 Channel
    vec3 material_emissive = texture(material.emission, fs_in.tex_coord).rgb;
    
    // Initialize the Color of the Pixel to Black
    vec3 color = vec3(0.0);
    //2. Lighting Calculations
    vec3 ambient = ambient_light * material_ambient;
    //Add Ambient Light to the Final Color
    color += ambient;
    //Add Emissive Light to the Final Color
    color += material_emissive;
    //Iterate over all the Lights
    for(int i = 0; i < light_count; i++){
        Light light = lights[i];

        vec3 light_direction;
        float attenuation = 1.0;

        if (light.type == DIRECTIONAL){
            //Directional Light [Light Direction is opposite to the Direction]
            light_direction = -light.direction;
        } else {
            //Point Light or Spot Light
            //Light direction is from the light to the fragment
            vec3 frag_light_vector = light.position - fs_in.world_position; // Vector from the Fragment to the Light Source

            float distance = length(frag_light_vector); // Distance
            light_direction = frag_light_vector / distance; // Normalize the Light Direction
            // Compute Attenuation [1.0 / (c * 1 + l * d + q * d^2)] (Distance)
            attenuation = 1.0 / dot(light.attenuation, vec3(1.0, distance, distance * distance));
            if(light.type == SPOT){
                // Comoute Cone Attenuation
                float theta_s = acos(dot(light.direction, -light_direction));
                attenuation *= smoothstep(light.outer_cone_angle, light.inner_cone_angle, theta_s);
            }
        }
        // Diffuse Component [Diffuse = Kd * Id * Max(0,l.n) ]
        float lambert = max(0.0,dot(normal, light_direction)); // Lambert's Cosine Law
        // vec3 diffuse = light.color * material.diffuse * lambert;
        vec3 diffuse = light.color * material_diffuse * lambert;

        // Specular Component [Specular = Ks * Is * Max(0, (r.v))^alpha]
        vec3 r = reflect(-light_direction, normal); // both light_direction and normal are normalized vectors ---> r is also normalized
        // float phong = pow(max(0.0, dot(r,view)), material.alpha); // Note: r and view must be normalized :D  or else the result will be wrong <3
        float phong = pow(max(0.0, dot(r,view)), material_shininess); // Note: r and view must be normalized :D  or else the result will be wrong <3
        vec3 specular = light.color * material_specular * phong;


        // Add Diffuse and Specular Components of the current Light Source
        color += (diffuse + specular) * attenuation;
    }   
    
    // Set the color of the pixel
    vec4 tex_color = texture(material.albedo, fs_in.tex_coord);
    frag_color = vec4(color, tex_color.a);  

}