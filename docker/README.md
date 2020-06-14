### Build the docker image

Under the project folder of fmm, run this command

```
docker build -f docker/Dockerfile . -t fmm:0.1.0
```

### Open bash in the docker image

```
docker run -i -t fmm:0.1.0 /bin/bash
```
