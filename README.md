# NeuralDrumLive Plugin 

Fork of Fyfe's [NeuralDrum-Live](https://github.com/Fyfe93/neurodrum-live) plugin, which is a JUCE-based plugin.

The plugin uses a generative NN model to create drum patterns using a number of descriptors. 

The model used is based on the following paper: 

```bibtex
@misc{ramires2020neuralpercussivesynthesisparameterised,
      title={Neural Percussive Synthesis Parameterised by High-Level Timbral Features}, 
      author={António Ramires and Pritish Chandna and Xavier Favory and Emilia Gómez and Xavier Serra},
      year={2020},
      eprint={1911.11853},
      archivePrefix={arXiv},
      primaryClass={eess.AS},
      url={https://arxiv.org/abs/1911.11853}, 
}
```

# Modifications of Fyfe's Plugin

- Added a CMake build system to the plugin.
- Uses static linking of ONNXRuntime using the prebuilt binaries provided by [Fangjun Kuang](https://github.com/csukuangfj/onnxruntime-libs)

# Compatibility

- [x] MacOS (ARM)
- [ ] MacOS (Intel)
- [ ] Linux
- [ ] Windows