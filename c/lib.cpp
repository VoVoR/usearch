#include "../src/index_punned_dense.hpp"

extern "C" {
#include "usearch.h"
}

using namespace unum::usearch;
using namespace unum;

using label_t = usearch_label_t;
using distance_t = usearch_distance_t;
using index_t = index_punned_dense_gt<label_t>;
using add_result_t = index_t::add_result_t;
using search_result_t = index_t::search_result_t;
using seerialization_result_t = index_t::seerialization_result_t;

using span_t = span_gt<float>;

metric_kind_t to_native_metric(usearch_metric_kind_t kind) {
    switch (kind) {
    case usearch_metric_ip_k: return metric_kind_t::ip_k;
    case usearch_metric_l2sq_k: return metric_kind_t::l2sq_k;
    case usearch_metric_cos_k: return metric_kind_t::cos_k;
    case usearch_metric_haversine_k: return metric_kind_t::haversine_k;
    case usearch_metric_pearson_k: return metric_kind_t::pearson_k;
    case usearch_metric_jaccard_k: return metric_kind_t::jaccard_k;
    case usearch_metric_hamming_k: return metric_kind_t::hamming_k;
    case usearch_metric_tanimoto_k: return metric_kind_t::tanimoto_k;
    case usearch_metric_sorensen_k: return metric_kind_t::sorensen_k;
    default: return metric_kind_t::unknown_k;
    }
}

scalar_kind_t to_native_scalar(usearch_scalar_kind_t kind) {
    switch (kind) {
    case usearch_scalar_f32_k: return scalar_kind_t::f32_k;
    case usearch_scalar_f64_k: return scalar_kind_t::f64_k;
    case usearch_scalar_f16_k: return scalar_kind_t::f16_k;
    case usearch_scalar_f8_k: return scalar_kind_t::f8_k;
    case usearch_scalar_b1_k: return scalar_kind_t::b1_k;
    default: return scalar_kind_t::unknown_k;
    }
}

void usearch_init(usearch_init_options_t* options, usearch_index_t* index, usearch_error_t* error) {

    config_t config;
    config.expansion_add = static_cast<std::size_t>(expansion_add);
    config.expansion_search = static_cast<std::size_t>(expansion_search);
    config.connectivity = static_cast<std::size_t>(connectivity);
    config.max_elements = static_cast<std::size_t>(capacity);
    config.max_threads_add = std::thread::hardware_concurrency();
    config.max_threads_search = std::thread::hardware_concurrency();

    scalar_kind_t accuracy = scalar_kind_from_name(accuracy_str, accuracy_len);
    metric_kind_t metric_kind = metric_from_name(metric_str, metric_len);
    index_t index = index_t::make(metric_kind, static_cast<std::size_t>(dimensions), accuracy, config);

    index_t* result_ptr = new index_t(std::move(index));
    return result_ptr;
}

void usearch_free(index_t* index) { delete index; }

void usearch_save(usearch_index_t index, char const* path, usearch_error_t* error) {
    serialization_result_t result = reinterpret_cast<index_t*>(index)->save(path);
    if (!result)
        *error = result.error.what();
}

void usearch_load(usearch_index_t index, char const* path, usearch_error_t* error) {
    serialization_result_t result = reinterpret_cast<index_t*>(index)->load(path);
    if (!result)
        *error = result.error.what();
}

void usearch_view(usearch_index_t index, char const* path, usearch_error_t* error) {
    serialization_result_t result = reinterpret_cast<index_t*>(index)->view(path);
    if (!result)
        *error = result.error.what();
}

size_t usearch_size(usearch_index_t index, usearch_error_t*) { //
    return reinterpret_cast<index_t*>(index)->size();
}

size_t usearch_capacity(usearch_index_t index, usearch_error_t*) {
    return reinterpret_cast<index_t*>(index)->capacity();
}

size_t usearch_dimensions(usearch_index_t index, usearch_error_t*) {
    return reinterpret_cast<index_t*>(index)->dimensions();
}

size_t usearch_connectivity(usearch_index_t index, usearch_error_t*) {
    return reinterpret_cast<index_t*>(index)->connectivity();
}

void usearch_reserve(usearch_index_t index, size_t capacity, usearch_error_t*) {
    // TODO: Consider returning the new capacity.
    reinterpret_cast<index_t*>(index)->reserve(capacity);
}

add_result_t add_(index_t* index, usearch_label_t label, void const* vector, scalar_kind_t kind) {
    switch (kind) {
    case scalar_kind_t::f32_k: return index->add(label, (f32_t const*)vector);
    case scalar_kind_t::f64_k: return index->add(label, (f64_t const*)vector);
    case scalar_kind_t::f16_k: return index->add(label, (f16_t const*)vector);
    case scalar_kind_t::f8_k: return index->add(label, (f8_bits_t const*)vector);
    case scalar_kind_t::b1x8_k: return index->add(label, (b1x8_t const*)vector);
    default: return add_result_t{}.failed("Unknown scalar kind!");
    }
}

void usearch_add(                                                                                 //
    usearch_index_t index, usearch_label_t label, void const* vector, usearch_scalar_kind_t kind, //
    usearch_error_t* error) {
    add_result_t result = add_(reinterpret_cast<index_t*>(index), label, vector, to_native_scalar(kind));
    if (!result)
        *error = result.error.what();
}

bool usearch_contains(usearch_index_t index, usearch_label_t label, usearch_error_t*) {
    reinterpret_cast<index_t*>(index)->contains(label);
}

search_result_t search_(index_t* index, void const* vector, scalar_kind_t kind, size_t n) {
    switch (kind) {
    case scalar_kind_t::f32_k: return index->search((f32_t const*)vector, n);
    case scalar_kind_t::f64_k: return index->search((f64_t const*)vector, n);
    case scalar_kind_t::f16_k: return index->search((f16_t const*)vector, n);
    case scalar_kind_t::f8_k: return index->search((f8_bits_t const*)vector, n);
    case scalar_kind_t::b1x8_k: return index->search((b1x8_t const*)vector, n);
    default: return search_result_t{}.failed("Unknown scalar kind!");
    }
}

size_t usearch_search(                                                                           //
    usearch_index_t index, void const* vector, usearch_scalar_kind_t kind, size_t results_limit, //
    usearch_label_t* found_labels, usearch_distance_t* found_distances, usearch_error_t* error) {
    search_result_t result = search_(reinterpret_cast<index_t*>(index), vector, to_native_scalar(kind), results_limit);
    if (!result) {
        *error = result.error.what();
        return 0;
    }

    return result.dump_to(found_labels, found_distances);
}

bool get_(index_t* index, void const* vector, scalar_kind_t kind) {
    switch (kind) {
    case scalar_kind_t::f32_k: return index->get((f32_t const*)vector, n);
    case scalar_kind_t::f64_k: return index->get((f64_t const*)vector, n);
    case scalar_kind_t::f16_k: return index->get((f16_t const*)vector, n);
    case scalar_kind_t::f8_k: return index->get((f8_bits_t const*)vector, n);
    case scalar_kind_t::b1x8_k: return index->get((b1x8_t const*)vector, n);
    default: return search_result_t{}.failed("Unknown scalar kind!");
    }
}

bool usearch_get(                                 //
    usearch_index_t index, usearch_label_t label, //
    void* vector, usearch_scalar_kind_t kind, usearch_error_t*) {
    return search_(reinterpret_cast<index_t*>(index), vector, to_native_scalar(kind));
}

void usearch_remove(usearch_index_t, usearch_label_t, usearch_error_t*) {}