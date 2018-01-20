
# Locates the tensorFlow library and include directories.

include(FindPackageHandleStandardArgs)
unset(TENSORFLOW_FOUND)

find_path(TensorFlow_INCLUDE_DIR
        NAMES
        tensorflow/core
        tensorflow/cc
        third_party
        HINTS
        /usr/local/include/google/tensorflow
        /usr/include/google/tensorflow
        ~/.local/lib/python3.5/site-packages/tensorflow/include)

find_path(TensorFlow_LIBRARY_PATH NAMES libtensorflow_framework.so
        HINTS
        /usr/lib
        /usr/local/lib
        ~/.local/lib/python3.5/site-packages/tensorflow)

# set TensorFlow_FOUND
find_package_handle_standard_args(TensorFlow DEFAULT_MSG TensorFlow_INCLUDE_DIR TensorFlow_LIBRARY_PATH)

# set external variables for usage in CMakeLists.txt
if(TENSORFLOW_FOUND)
#        -L${TensorFlow_LIBRARY_PATH}/python/ -l:_pywrap_tensorflow_internal.so
#        -L${TensorFlow_LIBRARY_PATH}/python/framework/ -l:fast_tensor_util.so
#        -L${TensorFlow_LIBRARY_PATH}/contrib/batching/python/ops/ -l:_batch_ops.so
    set(TensorFlow_LIBRARIES ${TensorFlow_LIBRARY_PATH}/libtensorflow_framework.so
        -L${TensorFlow_LIBRARY_PATH}/contrib/fused_conv/python/ops/ -l:_fused_conv2d_bias_activation_op.so
        -L${TensorFlow_LIBRARY_PATH}/contrib/tensor_forest/python/ops/ -l:_stats_ops.so
        -L${TensorFlow_LIBRARY_PATH}/contrib/tensor_forest/python/ops/ -l:_tensor_forest_ops.so
        -L${TensorFlow_LIBRARY_PATH}/contrib/tensor_forest/python/ops/ -l:_model_ops.so
        ${TensorFlow_LIBRARY_PATH}/contrib/tensor_forest/libforestprotos.so
        -L${TensorFlow_LIBRARY_PATH}/contrib/tensor_forest/hybrid/python/ops/ -l:_training_ops.so
        ${TensorFlow_LIBRARY_PATH}/contrib/factorization/python/ops/lib_factorization_ops.so
        -L${TensorFlow_LIBRARY_PATH}/contrib/factorization/python/ops/ -l:_factorization_ops.so
        -L${TensorFlow_LIBRARY_PATH}/contrib/factorization/python/ops/ -l:_clustering_ops.so
        -L${TensorFlow_LIBRARY_PATH}/contrib/seq2seq/python/ops/ -l:_beam_search_ops.so
        -L${TensorFlow_LIBRARY_PATH}/contrib/nccl/python/ops/ -l:_nccl_ops.so
        -L${TensorFlow_LIBRARY_PATH}/contrib/text/python/ops/ -l:_skip_gram_ops.so
        -L${TensorFlow_LIBRARY_PATH}/contrib/image/python/ops/ -l:_image_ops.so
        -L${TensorFlow_LIBRARY_PATH}/contrib/image/python/ops/ -l:_distort_image_ops.so
        -L${TensorFlow_LIBRARY_PATH}/contrib/image/python/ops/ -l:_single_image_random_dot_stereograms.so
        -L${TensorFlow_LIBRARY_PATH}/contrib/boosted_trees/python/ops/ -l:_boosted_trees_ops.so
        -L${TensorFlow_LIBRARY_PATH}/contrib/memory_stats/python/ops/ -l:_memory_stats_ops.so
        -L${TensorFlow_LIBRARY_PATH}/contrib/resampler/python/ops/ -l:_resampler_ops.so
        -L${TensorFlow_LIBRARY_PATH}/contrib/tpu/python/ops/ -l:_tpu_ops.so
        -L${TensorFlow_LIBRARY_PATH}/contrib/layers/python/ops/ -l:_sparse_feature_cross_op.so
        -L${TensorFlow_LIBRARY_PATH}/contrib/nearest_neighbor/python/ops/ -l:_nearest_neighbor_ops.so
        -L${TensorFlow_LIBRARY_PATH}/contrib/framework/python/ops/ -l:_variable_ops.so
        -L${TensorFlow_LIBRARY_PATH}/contrib/reduce_slice_ops/python/ops/ -l:_reduce_slice_ops.so
        -L${TensorFlow_LIBRARY_PATH}/contrib/cudnn_rnn/python/ops/ -l:_cudnn_rnn_ops.so
        -L${TensorFlow_LIBRARY_PATH}/contrib/ffmpeg/ -l:ffmpeg.so
        -L${TensorFlow_LIBRARY_PATH}/contrib/rnn/python/ops/ -l:_lstm_ops.so
        -L${TensorFlow_LIBRARY_PATH}/contrib/rnn/python/ops/ -l:_gru_ops.so
        -L${TensorFlow_LIBRARY_PATH}/contrib/input_pipeline/python/ops/ -l:_input_pipeline_ops.so
        -Wl,-rpath=${TensorFlow_LIBRARY_PATH}/contrib/fused_conv/python/ops/
        -Wl,-rpath=${TensorFlow_LIBRARY_PATH}/contrib/tensor_forest/python/ops/
        -Wl,-rpath=${TensorFlow_LIBRARY_PATH}/contrib/tensor_forest/
        -Wl,-rpath=${TensorFlow_LIBRARY_PATH}/contrib/tensor_forest/hybrid/python/ops/
        -Wl,-rpath=${TensorFlow_LIBRARY_PATH}/contrib/factorization/python/ops/
        -Wl,-rpath=${TensorFlow_LIBRARY_PATH}/contrib/seq2seq/python/ops/
        -Wl,-rpath=${TensorFlow_LIBRARY_PATH}/contrib/nccl/python/ops/
        -Wl,-rpath=${TensorFlow_LIBRARY_PATH}/contrib/text/python/ops/
        -Wl,-rpath=${TensorFlow_LIBRARY_PATH}/contrib/image/python/ops/
        -Wl,-rpath=${TensorFlow_LIBRARY_PATH}/contrib/boosted_trees/python/ops/
        -Wl,-rpath=${TensorFlow_LIBRARY_PATH}/contrib/memory_stats/python/ops/
        -Wl,-rpath=${TensorFlow_LIBRARY_PATH}/contrib/resampler/python/ops/
        -Wl,-rpath=${TensorFlow_LIBRARY_PATH}/contrib/tpu/python/ops/
        -Wl,-rpath=${TensorFlow_LIBRARY_PATH}/contrib/layers/python/ops/
        -Wl,-rpath=${TensorFlow_LIBRARY_PATH}/contrib/nearest_neighbor/python/ops/
        -Wl,-rpath=${TensorFlow_LIBRARY_PATH}/contrib/framework/python/ops/
        -Wl,-rpath=${TensorFlow_LIBRARY_PATH}/contrib/reduce_slice_ops/python/ops/
        -Wl,-rpath=${TensorFlow_LIBRARY_PATH}/contrib/cudnn_rnn/python/ops/
        -Wl,-rpath=${TensorFlow_LIBRARY_PATH}/contrib/ffmpeg/
        -Wl,-rpath=${TensorFlow_LIBRARY_PATH}/contrib/rnn/python/ops/
        -Wl,-rpath=${TensorFlow_LIBRARY_PATH}/contrib/input_pipeline/python/ops/)
    set(TensorFlow_INCLUDE_DIRS ${TensorFlow_INCLUDE_DIR} ${TensorFlow_INCLUDE_DIR}/external/nsync/public)
endif()


# hide locals from GUI
mark_as_advanced(TensorFlow_INCLUDE_DIR TensorFlow_LIBRARY_PATH)
