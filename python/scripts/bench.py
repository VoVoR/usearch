import os
import logging
import itertools
from typing import List, Optional, Iterable
from dataclasses import asdict

import numpy as np
import pandas as pd

from usearch.index import Index, Label
from usearch.eval import Evaluation, Dataset, AddTask
from usearch.index import (
    DEFAULT_CONNECTIVITY,
    DEFAULT_EXPANSION_ADD,
    DEFAULT_EXPANSION_SEARCH,
)


def bench_speed(
    eval: Evaluation,
    connectivity: int = DEFAULT_CONNECTIVITY,
    expansion_add: int = DEFAULT_EXPANSION_ADD,
    expansion_search: int = DEFAULT_EXPANSION_SEARCH,
    jit: bool = False,
    train: bool = False,
) -> pd.DataFrame:

    # Build various indexes:
    indexes = []
    jit_options = [False]
    if jit:
        jit_options.append(jit)
    dtype_options = ['f32', 'f16', 'f8']
    for jit, dtype in itertools.product(jit_options, dtype_options):

        index = Index(
            ndim=eval.ndim,
            jit=jit,
            dtype=dtype,
            expansion_add=expansion_add,
            expansion_search=expansion_search,
            connectivity=connectivity,
            path='USearch' + ['', '+JIT'][jit] + ':' + dtype,
        )
        
        # Skip the cases, where JIT-ing is impossible
        if jit and not index.jit:
            continue
        indexes.append(index)

    # Add FAISS indexes to the mix:
    try:
        from index_faiss import IndexFAISS, IndexQuantizedFAISS
        indexes.append(IndexFAISS(
            ndim=eval.ndim,
            expansion_add=expansion_add,
            expansion_search=expansion_search,
            connectivity=connectivity,
            path='FAISS:f32',
        ))
        if train:
            indexes.append(IndexQuantizedFAISS(
                train=eval.tasks[0].vectors,
                expansion_add=expansion_add,
                expansion_search=expansion_search,
                connectivity=connectivity,
                path='FAISS+IVFPQ:f32',
            ))
    except (ImportError, ModuleNotFoundError):
        pass

    # Time to evaluate:
    results = [eval(index) for index in indexes]
    return pd.DataFrame({
        'names': [i.path for i in indexes],
        'add_per_second': [x['add_per_second'] for x in results],
        'search_per_second': [x['search_per_second'] for x in results],
        'recall_at_one': [x['recall_at_one'] for x in results],
    })


def bench_params(
    count: int = 1_000_000,
    connectivities: int = range(10, 20),
    dimensions: List[int] = [
        2, 3, 4, 8, 16, 32, 96, 100,
        256, 384, 512, 768, 1024, 1536],

    expansion_add: int = DEFAULT_EXPANSION_ADD,
    expansion_search: int = DEFAULT_EXPANSION_SEARCH,
) -> pd.DataFrame:
    """Measures indexing speed for different dimensionality vectors.

    :param count: Number of vectors, defaults to 1_000_000
    :type count: int, optional
    """

    results = []
    for connectivity, ndim in itertools.product(connectivities, dimensions):

        task = AddTask(
            labels=np.arange(count, dtype=Label),
            vectors=np.random.rand(count, ndim).astype(np.float32),
        )
        index = Index(
            ndim=ndim,
            connectivity=connectivity,
            expansion_add=expansion_add,
            expansion_search=expansion_search,
        )
        result = asdict(task(index))
        result['ndim'] = dimensions
        result['connectivity'] = connectivity
        results.append(result)

    # return self._execute_tasks(
    #     tasks,
    #     title='HNSW Indexing Speed vs Vector Dimensions',
    #     x='ndim', y='add_per_second', log_x=True,
    # )
    return pd.DataFrame(results)
