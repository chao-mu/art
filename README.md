# art

A collection of small art projects in separate code bases. At some point I may
put common components in a library, but for now I'd like to keep them separate.

[https://hackpoetic.com](https://hackpoetic.com)

## Index

[00001](00001) - Melting/distorting skull and cross bones with RGB channels having separate velocities (GLSL/C++)

## Rendering

To record video output, instead of mucking with libav I just use apitrace and ffmpeg.

(see [https://github.com/apitrace/apitrace/blob/master/docs/USAGE.markdown](https://github.com/apitrace/apitrace/blob/master/docs/USAGE.markdown))

To install on Ubuntu:

```
sudo apt-get install apitrace ffmpeg
```

And then to record the trace (creating in this case art.trace because of the name of the binary)

```
apitrace trace --api gl build/art -i skull-and-crossbones.jpg
```

Or use the helper in this project
```
bin/gltrace art.trace <some command and args>
```

A barebones command to convert this trace to a video would be as follows:

```
apitrace dump-images -o - art.trace \
    | ffmpeg -r 30 -f image2pipe -vcodec ppm -i pipe: -vcodec mpeg4 -y output.mp4
```

However, this is quite low quality. bin/trace2insta will convert trace files to videos worthy of uploading to instagram.

```
bin/trace-to-instagram.sh input.trace output.mp4
```
