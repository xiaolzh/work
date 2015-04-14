#include "layer_header.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/// check the data is legal or not, 0 : legal, n : illegal, -1 : wrong
/// here only check the field which exist, if not, we assume it is ok
int check_data_legal(LAYER_FIELD* field_values) {
    int is_publication = -1;
    if ( NULL != field_values->is_publication )
        is_publication = atoi(field_values->is_publication);
    /// - check the product_name
    if ( NULL != field_values->product_name_status ) {
        if( 0 == atoi(field_values->product_name_status)) {
            /// product_name is illegal
            return NAME_ILLEGAL;
        }
    }
    /// - check the display_status
    if ( NULL != field_values->display_status ) {
        int display_status = atoi(field_values->display_status);
        if ( 0 != display_status ) {
            int product_medium = 22;
            if ( NULL != field_values->product_medium )
                product_medium = atoi(field_values->product_medium);
            if ( ! ( product_medium == 22 && display_status != 1 ) ) {
                /// display_status is illegal
                return DISPLAY_ILLEGAL;
            }
        }
    }
    /// - check the product_type
    if ( NULL != field_values->product_type ) {
        int type = atoi(field_values->product_type);
        if ( 1 == is_publication ) {
            if ( type == 99 || type == 1 ) 
                /// product_type is illegal
                return TYPE_ILLEGAL;
        } else if( 0 == is_publication) {
            if ( type != 0 && type != 9 ) 
                /// product_type is illegal
                return TYPE_ILLEGAL;
        }
    }
    /// - check the is_catalog_product
    if ( NULL != field_values->is_catalog_product 
        || NULL != field_values->main_product_id ) {
        if ( 0 == is_publication ) {
            int is_catalog_product = 0, main_product_id = 0;
            if ( NULL != field_values->is_catalog_product )
                is_catalog_product = atoi(field_values->is_catalog_product);
            if ( NULL != field_values->main_product_id )
                main_product_id = atoi(field_values->main_product_id);
            if ( is_catalog_product != 1 ) {
                if ( 0 != is_catalog_product || 0 != main_product_id ) 
                    /// catalog is illegal
                    return CATALOG_ILLEGAL;
            }
        }
    }
    /// - check the dd_sale_price
    if ( NULL != field_values->dd_sale_price ) {
        if ( 0 == is_publication ) {
            if ( atoi(field_values->dd_sale_price) <= 0 )
                /// price is illegal
                return PRICE_ILLEGAL;
        }
    }
    /// - check the stock_status
    if ( NULL != field_values->stock_status ) {
        bool is_ok = false;
        if ( 1 == is_publication ) is_ok = true;
        if ( ! is_ok && atoi(field_values->stock_status) > 0 ) is_ok = true;
        if ( ! is_ok ) {
            if ( NULL == field_values->pre_sale || 
                0 != atoi(field_values->pre_sale) ) 
                is_ok = true;
        }
        if ( ! is_ok ) {
            int is_share_product = 1;
            int shop_info_status = 1;
            if ( NULL != field_values->is_share_product )
                is_share_product = atoi(field_values->is_share_product);
            if ( NULL != field_values->shop_info_status )
                shop_info_status = atoi(field_values->shop_info_status);
            if ( 1 == is_share_product && 1 == shop_info_status )
                is_ok = true;
        }
        if ( ! is_ok ) 
            /// stock_status is illegal
            return STOCK_ILLEGAL;
    }
    /// - check the category_id
    if ( NULL != field_values->category_id ) {
        if ( 0 == is_publication ) {
            int cat_id = atoi(field_values->category_id);
            if ( cat_id <= 4000000 || cat_id == 4003973 )
                /// category_id is illegal
                return CATEGORY_ILLEGAL;
        }
    }
    /// - check the shop_id
    if ( NULL != field_values->shop_id ) {
        if ( 1 == is_publication ) {
            int shop_id = atoi(field_values->shop_id);
            if ( 0 != shop_id ) {
                int is_publish = 1;
                if ( NULL != field_values->is_publish )
                    is_publish = atoi(field_values->is_publish);
                if ( ! (shop_id > 0 && is_publish == 1) )
                    /// shop_id is illegal
                    return SHOP_ILLEGAL;
            }
        }
    }
    /// - check the product_medium
    if ( NULL != field_values->product_medium ) {
        if ( 1 == is_publication ) {
            if ( atoi(field_values->product_medium) == 21 ) 
                /// product_medium is illegal
                return MEDIUM_ILLEGAL;
        }
    }
    return DATA_OK;
}

#ifdef UNIT_TEST
#include <iostream>
using namespace std;

bool test_business_layer() {
    cout<<"Unit test - business_layer"<<endl;
    {
        bool ret = true;
        cout<<"usecase 1: check_data_legal"<<endl;
        LAYER_FIELD fields = {0};
        ret &= ( 0 == check_data_legal(&fields) );
        LAYER_FIELD fields2 = {
            "legal",   // product_name
            "2",       // display_status 
            "22",      // product_medium
            NULL,      // product_type
            "1",       // is_publication
            "",        // is_catalog_product
            NULL,      // main_product_id
            NULL,      // dd_sale_price
            "0",       // stock_status
            NULL,      // is_share_product
            NULL,      // shop_info_status
            "4000001", // category_id
            "1",       // shop_id
            "1"        // is_publish
            };
        ret &= ( 0 == check_data_legal(&fields2) );
        cout<<boolalpha<<ret<<endl;
        if( !ret)
            return false;
    }
    cout<<"Done unit test - business_layer"<<endl;
    return true;
}

#ifdef TEST_BUSINESS_LAYER
int main() {
    if( ! test_business_layer()) 
        return -1;
    return 0;
}
#endif // TEST_BUSINESS_LAYER
#endif // UNIT_TEST

// I wanna sing a song for you, dead loop no escape
