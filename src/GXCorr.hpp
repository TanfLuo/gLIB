/// ============================================================================
/// File    : GXCorr.hpp
/// Version : 0.1
/// Date    : May 2021
/// Author  : Gino Francesco Bogo
/// License : MIT
/// ============================================================================

#ifndef GXCORR_HPP
#define GXCORR_HHP

#include <cmath> // sqrt
#include <fftw3.h>
#include <type_traits> // is_same_v

namespace gGXCorr {
    struct NORMAL {
        bool _;
    };
    struct SHIFTED {
        bool _;
    };

} // namespace gGXCorr

template <size_t NUM> class GXCorr {
    public:
    GXCorr() {
        m_plan_A = fftw_plan_dft_1d(X_NUM, m_signal_A, m_result_A, FFTW_FORWARD, FFTW_ESTIMATE);
        m_plan_B = fftw_plan_dft_1d(X_NUM, m_signal_B, m_result_B, FFTW_FORWARD, FFTW_ESTIMATE);
        m_plan_X = fftw_plan_dft_1d(X_NUM, m_signal_X, m_middle_X, FFTW_BACKWARD, FFTW_ESTIMATE);
    }

    ~GXCorr() {
        fftw_destroy_plan(m_plan_A);
        fftw_destroy_plan(m_plan_B);
        fftw_destroy_plan(m_plan_X);
    }

    template <typename T> bool SetSignalA(T *signal_buffer, size_t signal_length) {
        if (!(signal_length & 1) && (signal_length <= 2 * NUM)) {
            auto K{signal_length / 2};
            for (size_t i{0}; i < K; ++i) {
                m_signal_A[i][0] = signal_buffer[2 * i + 0];
                m_signal_A[i][1] = signal_buffer[2 * i + 1];
            }
            for (size_t i{K}; i < X_NUM; ++i) {
                m_signal_A[i][0] = 0;
                m_signal_A[i][1] = 0;
            }
            fftw_execute(m_plan_A);
            return true;
        }
        return false;
    }

    template <typename T> bool SetSignalB(T *signal_buffer, size_t signal_length) {
        if (!(signal_length & 1) && (signal_length <= 2 * NUM)) {
            auto K{signal_length / 2};
            for (size_t i{0}; i < K; ++i) {
                m_signal_B[i][0] = signal_buffer[2 * i + 0];
                m_signal_B[i][1] = signal_buffer[2 * i + 1];
            }
            for (size_t i{K}; i < X_NUM; ++i) {
                m_signal_B[i][0] = 0;
                m_signal_B[i][1] = 0;
            }
            fftw_execute(m_plan_B);
            return true;
        }
        return false;
    }

    template <typename OP> void Execute() {
        auto const Re{0};
        auto const Im{1};

        for (size_t i{0}; i < X_NUM; ++i) {
            m_signal_X[i][Re] = m_result_A[i][Re] * m_result_B[i][Re] + m_result_A[i][Im] * m_result_B[i][Im];
            m_signal_X[i][Im] = m_result_A[i][Im] * m_result_B[i][Re] - m_result_A[i][Re] * m_result_B[i][Im];
        }
        fftw_execute(m_plan_X);

        if constexpr (std::is_same_v<OP, gGXCorr::NORMAL>) {
            for (size_t i{0}; i < X_NUM; ++i) {
                m_result_X[i][Re] = m_middle_X[i][Re];
                m_result_X[i][Im] = m_middle_X[i][Im];
            }
        }

        if constexpr (std::is_same_v<OP, gGXCorr::SHIFTED>) {
            auto K{X_NUM / 2};
            for (size_t i{0}; i <= K; ++i) {
                m_result_X[K + i][Re] = m_middle_X[i][Re];
                m_result_X[K + i][Im] = m_middle_X[i][Im];
            }
            for (size_t i{0}; i < K; ++i) {
                m_result_X[i][Re] = m_middle_X[K + 1 + i][Re];
                m_result_X[i][Im] = m_middle_X[K + 1 + i][Im];
            }
        }
    }

    void MaxPeak(double *value, size_t *index) {
        auto const Re{0};
        auto const Im{1};

        *value = 0;
        *index = 0;
        for (size_t i{0}; i < X_NUM; ++i) {
            auto peak{m_result_X[i][Re] * m_result_X[i][Re] + m_result_X[i][Im] * m_result_X[i][Im]};
            if (peak > *value) {
                *value = peak;
                *index = i;
            }
        }
        *value = std::sqrt(*value);
    }

    auto signal_A() {
        return m_signal_A;
    }

    auto signal_B() {
        return m_signal_B;
    }

    auto result_X() {
        return m_result_X;
    }

    auto length() {
        return X_NUM;
    }

    private:
    const size_t X_NUM{2 * NUM - 1};

    fftw_plan m_plan_A{nullptr};
    fftw_plan m_plan_B{nullptr};
    fftw_plan m_plan_X{nullptr};

    fftw_complex m_signal_A[2 * NUM - 1];
    fftw_complex m_result_A[2 * NUM - 1];
    fftw_complex m_signal_B[2 * NUM - 1];
    fftw_complex m_result_B[2 * NUM - 1];
    fftw_complex m_signal_X[2 * NUM - 1];
    fftw_complex m_middle_X[2 * NUM - 1];
    fftw_complex m_result_X[2 * NUM - 1];
};

#endif // GXCORR_HPP