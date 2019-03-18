# 00001

I am recursively modifying an image with a fragment shader (see [frag.glsl](frag.glsl)).
For this modification I am using a vector field generated from perlin noise. Each vector is
then multiplied with the current coordinates to use in a texture fetch.

To run this, build img-frag in the root of this repository and run (within this directory):

```
img-frag -i skull-and-crossbones.jpg
```

## Preview

![preview 2](preview-2.png?raw=true)

![preview 1](preview-1.png?raw=true)

## Sources

skull-and-crossbones.jpg - https://www.publicdomainpictures.net/en/view-image.php?image=130392&picture=skull-and-crossbones
