// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.

#pragma once

// cpp headers
#include <random>
#include <memory>
// ntl headers
#include "ntlVersionConversion.hpp"

namespace ntl {

    /// Wraps the somewhat unweildy STL random routines for common use cases
    ///
    /// This random number generator makes several important assumptions:
    ///   - Cryptographic-level randomness is unnecessary
    ///   - Moderately high space usage is okay (an instance takes ~5kb)
    ///   - Seeding with only an unsigned long's worth of entropy is okay
    ///
    /// These assumptions are perfectly okay in most common cases. If any of them
    /// are invalid, use either the windows cryptographic-quality random generation
    /// routines or STL <random> directly instead of this class.
    ///
    /// This class uses the STL's mersenne twister implementation internally, which means
    /// that this class requires considerable space (~5kb heap space), but random number
    /// generation is fast and provides fairly good random distributions (good
    /// enough for just about anything non-cryptographic)
    class RandomTwister {
    public:
        typedef std::mt19937 engine_type;

        /// Constructs the generator with an explicitly specified seed.
        /// This is usually unnecessary, since the default constructor will seed the generator
        /// with an appropriately random seed
        ///
        /// This allocates a large (5kb) chunk of heap memory and may throw std::bad_alloc
        explicit RandomTwister(unsigned long _seed);

        /// Seeds itself randomly with std::random_device
        ///
        /// This allocates a large (5kb) chunk of heap memory and may throw std::bad_alloc
        RandomTwister();

        /// Generates a new random integer in the range [lowerInclusiveBound, upperInclusiveBound].
        /// Each integer in the range is equally likely to be chosen.
        ///
        /// It's usually best to explicitly specify the template argument to this function - the compiler
        /// can surprise you if you let it choose what type to output.
        template <class IntegerT>
        IntegerT uniform_int(IntegerT _lowerInclusiveBound, IntegerT _upperInclusiveBound);

        /// Generates a new random floating-point number in the range [lowerInclusiveBound, upperInclusiveBound].
        ///
        /// The result is chosen according to a uniformaly random distribution of real numbers, not a uniformly
        /// random distribution of those numbers representable as RealTs. That is, even though a double can represent
        /// more distinct values in the range [0.0, 1.0] than it can in the range [99.0, 100.0], uniform_real(0.0, 100.0)
        /// will return a number in those two ranges equally often.
        template <class RealT>
        RealT uniform_real(RealT _lowerInclusiveBound, RealT _upperInclusiveBound);

        /// Generates a new random floating-point number chosen uniformly at random from the range [0.0, 1.0].
        double uniform_probability();

        /// Generates a new random double chosen randomly from a normal distribution with the characteristics
        /// given by the two parameters (by default, a standard normal distribution).
        double normal_real(double _distributionMean = 0.0, double _distributionSigma = 1.0);

        /// Seeds the generator manually.
        void seed(unsigned long _seed);

        // Enabling move, move assign, and swap
        RandomTwister(RandomTwister&& _other) NOEXCEPT;
        RandomTwister& operator=(RandomTwister&& _other) NOEXCEPT;
        void swap(RandomTwister& _other) NOEXCEPT;

        ~RandomTwister() = default;

        // Non-copyable mostly because instances are space-heavy (mt19937s are big)
        RandomTwister(const RandomTwister&) = delete;
        RandomTwister& operator=(const RandomTwister&) = delete;

    private:
        /// Keep the 5kb engine on the heap
        /// We can use unique_ptr because there's a header-level STLVER>=100 requirement
        std::unique_ptr<engine_type> engine;
    };

    // non-member namespace swap
    inline void swap(RandomTwister& lhs, RandomTwister& rhs) NOEXCEPT
    {
        lhs.swap(rhs);
    }


    // Implementation

    inline RandomTwister::RandomTwister(unsigned long seed) : 
        engine(new engine_type(seed))
    {
    }

    inline RandomTwister::RandomTwister() : 
        engine(new engine_type(std::random_device()()))
    {
    }

    inline RandomTwister::RandomTwister(RandomTwister&& other) NOEXCEPT :
        engine(std::move(other.engine))
    {
    }

    inline RandomTwister& RandomTwister::operator=(RandomTwister&& other) NOEXCEPT
    {
        RandomTwister temp(std::move(other));
        temp.swap(*this);
        return *this;
    }

    inline void RandomTwister::swap(RandomTwister& other) NOEXCEPT
    {
        using std::swap;
        swap(this->engine, other.engine);
    }

    template <class IntegerT>
    IntegerT RandomTwister::uniform_int(IntegerT lowerInclusiveBound, IntegerT upperInclusiveBound)
    {
        return std::uniform_int_distribution<IntegerT>(lowerInclusiveBound, upperInclusiveBound)(*engine);
    }

    template <class RealT>
    RealT RandomTwister::uniform_real(RealT lowerInclusiveBound, RealT upperInclusiveBound)
    {
        return std::uniform_real_distribution<RealT>(lowerInclusiveBound, upperInclusiveBound)(*engine);
    }

    inline double RandomTwister::uniform_probability()
    {
        return std::uniform_real_distribution<double>(0.0, 1.0)(*engine);
    }

    inline double RandomTwister::normal_real(double distributionMean, double distributionSigma)
    {
        return std::normal_distribution<double>(distributionMean, distributionSigma)(*engine);
    }

    inline void RandomTwister::seed(unsigned long _seed)
    {
        engine->seed(_seed);
    }

} // namespace ntl