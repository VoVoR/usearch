#pragma once

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSUInteger, USearchScalar) {
    USearchScalarF32,
    USearchScalarF16,
    USearchScalarF64,
    USearchScalarF8,
    USearchScalarB1
};

typedef NS_ENUM(NSUInteger, USearchMetric) {
    USearchMetricUnknown,
    USearchMetricIP,
    USearchMetricCos,
    USearchMetricL2sq,
    USearchMetricPearson,
    USearchMetricHaversine,
    USearchMetricJaccard,
    USearchMetricHamming,
    USearchMetricTanimoto,
    USearchMetricSorensen
};

typedef UInt32 USearchLabel;

API_AVAILABLE(ios(13.0), macos(10.15), tvos(13.0), watchos(6.0))
@interface USearchIndex : NSObject

@property (readonly) UInt32 dimensions;
@property (readonly) UInt32 connectivity;
@property (readonly) UInt32 expansionAdd;
@property (readonly) UInt32 expansionSearch;

@property (readonly) UInt32 length;
@property (readonly) UInt32 capacity;
@property (readonly) Boolean isEmpty;

- (instancetype)init NS_UNAVAILABLE;

/**
 * @brief Initializes a new index.
 * @param metric The distance function to compare the dis-similarity of vectors.
 * @param dimensions The number of dimensions planned for this index.
 * @param connectivity Number of connections per node in the proximity graph.
 * Higher connectivity improves accuracy, increases memory usage, and reduces construction speed.
 * @param quantization Accuracy of internal vector representations. Lower accuracy means higher speed.
 */
+ (instancetype)make:(USearchMetric)metric dimensions:(UInt32)dimensions connectivity:(UInt32)connectivity quantization:(USearchScalar)quantization NS_SWIFT_NAME(make(metric:dimensions:connectivity:quantization:));

/**
 * @brief Adds a labeled vector to the index.
 * @param vector Single-precision vector.
 */
- (void)addSingle:(USearchLabel)label
           vector:(Float32 const *_Nonnull)vector NS_SWIFT_NAME(addSingle(label:vector:));

/**
 * @brief Approximate nearest neighbors search.
 * @param vector Single-precision query vector.
 * @param count Upper limit on the number of matches to retrieve.
 * @param labels Optional output buffer for labels of approximate neighbors.
 * @param distances Optional output buffer for (increasing) distances to approximate neighbors.
 * @return Number of matches exported to `labels` and `distances`.
 */
- (UInt32)searchSingle:(Float32 const *_Nonnull)vector
                 count:(UInt32)count
                labels:(USearchLabel *_Nullable)labels
             distances:(Float32 *_Nullable)distances NS_SWIFT_NAME(searchSingle(vector:count:labels:distances:));

/**
 * @brief Adds a labeled vector to the index.
 * @param vector Double-precision vector.
 */
- (void)addDouble:(UInt32)label
           vector:(Float64 const *_Nonnull)vector NS_SWIFT_NAME(addDouble(label:vector:));

/**
 * @brief Approximate nearest neighbors search.
 * @param vector Double-precision query vector.
 * @param count Upper limit on the number of matches to retrieve.
 * @param labels Optional output buffer for labels of approximate neighbors.
 * @param distances Optional output buffer for (increasing) distances to approximate neighbors.
 * @return Number of matches exported to `labels` and `distances`.
 */
- (UInt32)searchDouble:(Float64 const *_Nonnull)vector
                 count:(UInt32)count
                labels:(USearchLabel *_Nullable)labels
             distances:(Float32 *_Nullable)distances NS_SWIFT_NAME(searchDouble(vector:count:labels:distances:));

/**
 * @brief Adds a labeled vector to the index.
 * @param vector Half-precision vector.
 */
- (void)addHalf:(UInt32)label
         vector:(void const *_Nonnull)vector NS_SWIFT_NAME(addHalf(label:vector:));

/**
 * @brief Approximate nearest neighbors search.
 * @param vector Half-precision query vector.
 * @param count Upper limit on the number of matches to retrieve.
 * @param labels Optional output buffer for labels of approximate neighbors.
 * @param distances Optional output buffer for (increasing) distances to approximate neighbors.
 * @return Number of matches exported to `labels` and `distances`.
 */
- (UInt32)searchHalf:(void const *_Nonnull)vector
               count:(UInt32)count
              labels:(USearchLabel *_Nullable)labels
           distances:(Float32 *_Nullable)distances NS_SWIFT_NAME(searchHalf(vector:count:labels:distances:));

/**
 * @brief Saves pre-constructed index to disk.
 */
- (void)save:(NSString *)path NS_SWIFT_NAME(save(path:));

/**
 * @brief Loads a pre-constructed index from index.
 */
- (void)load:(NSString *)path NS_SWIFT_NAME(load(path:));

/**
 * @brief Views a pre-constructed index from disk without loading it into RAM.
 *        Allows working with larger-than memory indexes and saving scarce
 *        memory on device in read-only workloads.
 */
- (void)view:(NSString *)path NS_SWIFT_NAME(view(path:));

/**
 * @brief Removes all the data from index, while preserving the settings.
 */
- (void)clear NS_SWIFT_NAME(clear());

@end

NS_ASSUME_NONNULL_END
