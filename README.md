# HeightMap
3D computer graphics project

Heightmaps generated with (black-white):
https://manticorp.github.io/unrealheightmap/

Textures used from:
https://3dtextures.me

CONTROLS:
W, A, S, D - Movement
Q - lower Y scale (only when splitting into chunks is off)
E - raise Y scale (only when splitting into chunks is off)
SPACE - go up
SHIFT - go down
LMB (HOLD) - Look around
I - Decrease terrain vertex density 
O - Original terrain vertex density
R - Toggle grid
P - Toggle splitting into chunks

# Idea:

Srpski:
Height mapa se ucitava sa slike. Visina vertex-a se racuna u odnosu na intenzitet pixela sa slike ( ref: https://blogs.igalia.com/itoral/2016/10/13/opengl-terrain-renderer-rendering-the-terrain-mesh/ ).
Height mapa je organizovana u chunk-ove koji sluze da bi se lakse implementirao LOD (level of detail).
Height mape razlicitih gustina se generisu na pocetku (iz chunkova), i prikazuju se prilikom renderovanja potrebni chunkovi (low/high res u zavisnosti od udaljenosti).
Chunkovi ciji centri su udaljeni za manje od proizvoljnog dxz od polozaja kamere ce biti u maksimalnoj gustini vrhova (vertices), dok ce ostali chunkovi imati manju gustinu vrhova.
Teksture - Teren ima vise tekstura u zavisnosti od visine (ref: https://www.mbsoftworks.sk/tutorials/opengl4/018-heightmap-pt3-multiple-layers/#google_vignette ).
1. nivo lava
2. nivo interpolacija izmedju lave i kamena
3. nivo kamen
Na slican nacin se interpoliraju i normal mape.
Normal mape su koriscene sa ciljem da povecaju foto-realisticnost i da pritom ne usloznjavaju kolicinu poligona i racunaju se pomocu tangent, bitangent i normalnog vektora (ref: https://learnopengl.com/Advanced-Lighting/Normal-Mapping ). Koristi se najjednostavniji Lambertov model za difuzno osvetljenje.
