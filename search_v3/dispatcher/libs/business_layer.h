/**  
 *   This header file defines the data structure of dangdang business.
 *
 */
#ifndef BUSINESS_LAYER_H
#define BUSINESS_LAYER_H

#ifdef __cplusplus
extern "C" {
#endif

enum FieldType {
    PRODUCT_NAME_STATUS      /// 'product_name_status'
    , DISPLAY_STATUS         /// 'display_status'
    , PRODUCT_MEDIUM         /// 'product_medium'
    , PRODUCT_TYPE           /// 'product_type'
    , IS_PUBLICATION         /// 'is_publication'
    , IS_CATALOG_PRODUCT     /// 'is_catalog_product'
    , MAIN_PRODUCT_ID        /// 'main_product_id'
    , DD_SALE_PRICE          /// 'dd_sale_price'
    , STOCK_STATUS           /// 'stock_status'
    , PRE_SALE               /// 'pre_sale'
    , IS_SHARE_PRODUCT       /// 'is_share_product'
    , SHOP_INFO_STATUS       /// 'shop_info_status' 
    , CATEGORY_ID            /// 'category_id'
    , SHOP_ID                /// 'shop_id'
    , IS_PUBLISH             /// 'is_publish'
    , FIELD_NUM              /// the number of fields 
};

static const char* FIELD_NAME_LIST[] = {
    "product_name_status",
    "display_status",
    "product_medium",
    "product_type",
    "is_publication",
    "is_catalog_product",
    "main_product_id",
    "dd_sale_price",
    "stock_status",
    "pre_sale",
    "is_share_product",
    "shop_info_status",
    "category_id",
    "shop_id",
    "is_publish"
};

/// the fields needed by layer
typedef struct st_layer_field {
    const char* product_name_status;
    const char* display_status;
    const char* product_medium;
    const char* product_type;
    const char* is_publication;
    const char* is_catalog_product;
    const char* main_product_id;
    const char* dd_sale_price;
    const char* stock_status;
    const char* pre_sale;
    const char* is_share_product;
    const char* shop_info_status;
    const char* category_id;
    const char* shop_id;
    const char* is_publish;
} LAYER_FIELD;

/// check_data_legal return type
enum enum_legal_type {
    DATA_OK = 0x0,
    NAME_ILLEGAL = 0x1,
    DISPLAY_ILLEGAL = 0x2,
    TYPE_ILLEGAL = 0x4,
    CATALOG_ILLEGAL = 0x8,
    PRICE_ILLEGAL = 0x10,
    STOCK_ILLEGAL = 0x20,
    CATEGORY_ILLEGAL = 0x40,
    SHOP_ILLEGAL = 0x80,
    MEDIUM_ILLEGAL = 0x100
};


/// check the data is legal or not, 0 : legal, n : illegal, -1 : wrong
/// here only check the field which exist, if not, we assume it is ok
//int check_data_legal(LAYER_FIELD* field_values);

#ifdef __cplusplus
}
#endif

#endif // ~>.!.<~
