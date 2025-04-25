/**
 * @file proj_context.h
 * @author
 * @brief
 * @version 0.1
 * @date 2023-04-09
 *
 * @copyright Copyright (c) 2025 宇宙サービスイノベーションラボ
 *
 */

#ifndef __PROJ_CONTEXT__
#define __PROJ_CONTEXT__

#include <proj.h>

#include <map>
#include <stdexcept>
#include <string>

#include "common/transformer.h"
#include "logging/logger.h"

namespace spatialid {

/**
 * @brief PROJコンテキストクラス
 *
 */
class ProjContext {
   private:
    PJ_CONTEXT *pj_ctx_;
    std::map<std::string, Transformer *> pj_list_;

   protected:
    /**
     * @brief Construct a new Proj Context object
     *
     */
    ProjContext() : pj_ctx_(nullptr) {
        pj_ctx_ = proj_context_create();
        if (!pj_ctx_) {
            throw std::invalid_argument("VALUE_CONVERT_ERROR");
        }
    }

    /**
     * @brief Destroy the Proj Context object
     *
     */
    ~ProjContext() {
        for (const auto &pj : pj_list_) {
            delete pj.second;
        }
        if (pj_ctx_) {
            proj_context_destroy(pj_ctx_);
        }
    }

   public:
    ProjContext(const ProjContext &) = delete;
    ProjContext(ProjContext &&) = delete;
    ProjContext &operator=(const ProjContext &) = delete;
    ProjContext &operator=(ProjContext &&) = delete;

    /**
     * @brief Get the Instance object
     *
     * @return ProjContext&
     */
    static ProjContext &GetInstance() {
        static ProjContext proj_context;
        return proj_context;
    }

    /**
     * @brief Get the Context object
     * 
     * @return PJ_CONTEXT* 
     */
    PJ_CONTEXT *GetContext() const { return pj_ctx_; }

    /**
     * @brief Get the Transformer object
     * 
     * @param from_crs 
     * @param to_crs 
     * @return Transformer* 
     */
    Transformer *GetTransformer(CRS from_crs, CRS to_crs) {
        std::string key_name;
        key_name.append(std::to_string(static_cast<int>(from_crs)));
        key_name.append("-");
        key_name.append(std::to_string(static_cast<int>(to_crs)));

        auto itr = pj_list_.find(key_name);
        if (itr != pj_list_.end()) {
            return itr->second;
        }

        Transformer *transformer = new Transformer(from_crs, to_crs);
        pj_list_[key_name] = transformer;
        return transformer;
    }
};

}  // namespace spatialid

#endif  // __PROJ_CONTEXT__
