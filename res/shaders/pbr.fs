#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

uniform sampler2D albedoMap;                                            //  all required material parameters
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

uniform vec3 lightPositions[4];                                         //  positions of all light sources
uniform vec3 lightColors[4];                                            //  colors of all light sources

uniform vec3 camPos;                                                    //  camera position value

const float myPi = 3.14159265359;

/*
    Normal distribution function

    Implementation of the Trowbridge-Reitz GGX normal distribution function
    Formula represents approximated ratio of microfacets aligned to halfway vector H
    (alpha^2) / (Pi * ((N dot H)^2 * (alpha^2 - 1) + 1)^2)

    Function requires normal and halfway vector as parameters
    Material roughness value is required as a third parameter
    Return value represents result of this distribution function as a float value
*/
float NormalDistributionFunction(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;                                    //  material roughness representation
    float aSquare = a * a;                                              //  we are working with square area

    float NdotH = max(dot(N, H), 0.0);                                  //  calculate dot product of normal and halfway vector
    float factor = NdotH * NdotH * (aSquare - 1.0) + 1.0;               //  multiply squared vector with roughness value

    return aSquare / (myPi * factor * factor);                          //  result is plane cut of the sphere area
}

/*
    Geometry function

    Combination of Schlick-Beckmann approximation and GGX
    Formula represents light rays energy lose caused by overshadowing of microfacets
    k = ((alpha + 1)^2) / 8
    (N dot V) / ((N dot V) * (1 - k) + k)

    First parameter is result of the dot product of normal and view vectors
    Material roughness value is required as a second parameter
    Return value represents final ratio of microfacets overshadowing
*/
float GeometryFunctionSchlick(float NdotV, float roughness)
{
    float roughPlus = roughness + 1.0;
    float k = (roughPlus * roughPlus) / 8.0;                            //  we need to calculate remapping of material roughness for direct lighting 

    return NdotV / (NdotV * (1.0 - k) + k);                             //  compute probability of microfacets overshadowing
}

/*
    Geometry function

    Smith’s method
    This method takes into account view direction(geometry obstruction) and light direction vector(geometry shadowing)
    Schlick(N, V, k) * Schlick(N, L, k)

    First parameter represents normal vector
    Second parameter represents view direction vector
    Third parameter represents light direction vector
    Return value represents combinated ratio of overshadowing and geometry obstruction
*/
float GeometryFunctionSmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);                                  //  calculate dot product of normal and view direction vector
    float NdotL = max(dot(N, L), 0.0);                                  //  calculate dot product of normal and light direction vector

    float obstruction = GeometryFunctionSchlick(NdotV, roughness);      //  calculate ratio value just for view direction
    float shadowing = GeometryFunctionSchlick(NdotL, roughness);        //  calculate ratio value just for light direction

    return shadowing * obstruction;                                     //  get product of calculated ratios
}

/*
    Fresnel Equation

    Fresnel-Schlick approximation of the Fresnel Equation
    This method calculates ratio of reflected light over the refracted light
    F0 + (1 - F0) * (1 - (N dot V))^5

    First parameter represents angle between normal and view vectors
    Second parameter represents base reflectivity of the surface
    Return value represents percentage of reflected light
*/
vec3 FresnelEquation(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);                  //  calculate percentage of reflected light
}

/*
    Get the normal vector from map

    It is required to calculate normal, tangent and bitangent vectors
*/
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;     //  get tangent normal vector from the normal texture map

    vec3 Q1 = dFdx(WorldPos);                                           //  get horizontal position in world space
    vec3 Q2 = dFdy(WorldPos);                                           //  get vertical position in world space
    vec2 st1 = dFdx(TexCoords);                                         //  get horizontal coordinate in texture
    vec2 st2 = dFdy(TexCoords);                                         //  get vertical coordinate in texture

    vec3 N = normalize(Normal);                                         //  calculate normal vector using normalization
    vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);                        //  calculate tangent vector and normalize it
    vec3 B = -normalize(cross(N, T));                                   //  calculate bitangent vector using the cross product and normalization
    mat3 TBN = mat3(T, B, N);                                           //  create matrix from calculated vectors

    return normalize(TBN * tangentNormal);                              //  calculate final normal vector using normalization
}

void main()
{       
    vec3 albedo = pow(texture(albedoMap, TexCoords).rgb, vec3(2.2));    //  get all required textures
    float metallic = texture(metallicMap, TexCoords).r;
    float roughness = texture(roughnessMap, TexCoords).r;
    float ao = texture(aoMap, TexCoords).r;

    vec3 N = getNormalFromMap();                                        //  get the normal vector from map
    vec3 V = normalize(camPos - WorldPos);                              //  calculate the view vector

    vec3 F0 = vec3(0.04);                                               //  calculate reflectance at normal incidence
    F0 = mix(F0, albedo, metallic);                                     //  use F0 of 0.04 for dia-electric and F0 for metal material

    vec3 Lo = vec3(0.0);                                                //  holds result of the reflectance equation, so it is sum of radiance values for all light sources

    for (int i = 0; i < 4; i++)                                         //  we need to handle all present light sources and calculate radiance
    {
        vec3 L = normalize(lightPositions[i] - WorldPos);               //  calculate light direction vector
        vec3 H = normalize(V + L);                                      //  calculate halfway vector

        float distance = length(lightPositions[i] - WorldPos);          //  get distance and attenuation values of the light source
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;                   //  calculate radiance vector of light
                                                                        //  we have to calculate Cook-Torrance BRDF
        float NDF = NormalDistributionFunction(N, H, roughness);        //  Normal distribution function
        float Geometry = GeometryFunctionSmith(N, V, L, roughness);     //  Geometry function for both view and light direction vectors
        vec3 kS = FresnelEquation(max(dot(H, V), 0.0), F0);             //  specular vector is equal to result of the Fresnel equation
        vec3 kD = vec3(1.0) - kS;                                       //  handle energy conservation principle and calculate diffuse component using difference operator
        kD *= 1.0 - metallic;                                           //  materials don't have diffuse lighting so use difference again

        float factor = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;  //  we need to use 0.001 value instead of zero to prevent divide by zero
        vec3 specular = (NDF * Geometry * kS) / factor;                 //  calculate specular vector

        float NdotL = max(dot(N, L), 0.0);                              //  we need to scale the light by NdotL

        Lo += (kD * albedo / myPi + specular) * radiance * NdotL;       //   outgoing radiance value needs to be added
    }   

    vec3 ambient = vec3(0.03) * albedo * ao;                            //  calculate ambient lighting vector from albedo and ambient occlusion maps

    vec3 color = ambient + Lo;                                          //  final color value is sum of ambient and radiance values
    color = color / (color + vec3(1.0));                                //  we need to use HDR tonemapping
    color = pow(color, vec3(1.0 / 2.2));                                //  now apply the gamma correction

    FragColor = vec4(color, 1.0);
}
