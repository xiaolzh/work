/*==============================================================*/
/*                   搜索主视图                                 */
/*==============================================================*/
USE prodviewdb;

DROP VIEW IF exists test_search_tmp2;

/*==============================================================*/
/* VIEW: test_search                                            */
/*==============================================================*/
/* TODO: index view 利用索引视图加速 */
CREATE VIEW test_search_tmp2 AS
SELECT
    /* ================ product_core_search ===================*/
    /* 商品ID 主键 */
    core.product_id AS product_id
    /* 商品名 */
    , core.product_name AS product_name
    /* 商品介质（商品类型标识） */
    , core.product_medium AS product_medium
    /* 分类ID */
    , core.category_id AS category_id
    /* 分类路径（多个） */
    , core.cat_paths AS cat_paths
    /* 分类路径名 */
    , core.cat_paths_name AS cat_paths_name
    /* 是否是图书，1是0否 */
    , core.is_publication AS is_publication
    /* 出版社 */
    , core.publisher AS publisher
    /* 作者名 */
    , core.author_name AS author_name
    /* 演唱者，CD等商品需要该字段? */
    , core.singer AS singer
    /* 制造商 */
    , core.conductor AS conductor
    /* 导演 */
    , core.director AS director
    /* 演员 */
    , core.actors AS actors
    /* 丛书名 */
    , core.series_name AS series_name
    /* 出版时间 */
    , core.publish_date AS publish_date
    /* 商品状态，0上架1下架2无用 */
    , core.display_status AS display_status
    /* 是否是预售商品，1是0否 */
    , core.pre_sale AS pre_sale
    /* 特殊商品，1是0否 */
    , core.special_sale AS special_sale
    /* 促销id */
    , core.promotion_id AS promotion_id
    /* 首次上架时间? */
    , core.first_input_date AS first_input_date
    /* 店铺id，0是当当 */
    , core.shop_id AS shop_id
    /* 促销开始时间 */
    , core.promo_start_date AS promo_start_date
    /* 促销结束时间 */
    , core.promo_end_date AS promo_end_date
    /* 促销类型 */
    , core.promotion_type AS promotion_type
    /* 是否为产品，1是0否 */
    , core.is_catalog_product AS is_catalog_product
    /* @TODO 不详 */
    , core.book_id AS book_id
    /* @TODO 不详 */
    , core.author_format AS author_format
    /* @TODO 不详 */
    , core.singer_format AS singer_format
    /* @TODO 不详 */
    , core.director_format AS director_format
    /* @TODO 不详 */
    , core.icon_flag_life_five_star AS icon_flag_life_five_star
    /* @TODO 不详 */
    , core.icon_flag_sole AS icon_flag_sole
    /* @TODO 不详 */
    , core.icon_flag_mall_new AS icon_flag_mall_new
    /* @TODO 不详 */
    , core.icon_flag_mall_hot AS icon_flag_mall_hot
    /* @TODO 不详 */
    , core.icon_flag_bang AS icon_flag_bang
    /* @TODO 不详 */
    , core.icon_flag_character AS icon_flag_character
    /* 副标题 */
    , core.subname AS subname
    /* 标签id? */
    , core.product_action_id AS product_action_id
    /* 标签开始时间 */
    , core.action_start_date AS action_start_date
    /* 标签结束时间 */
    , core.action_end_date AS action_end_date
    /* 总浏览数 */
    , core.total_review_count AS total_review_count
    /* 收藏数? */
    , core.favorite_count AS favorite_count
    /* 平均评分*2 */
    , core.score AS score
    /* 促销价 */
    , core.promo_saleprice AS promo_saleprice
    /* @TODO 不详 */
    , core.promotion_filt AS promotion_filt
    /* @TODO 不详 */
    , core.product_promotion_filt AS product_promotion_filt
    /* 是否有对应电子书？ */
    , core.is_has_ebook AS is_has_ebook
    /* 对应纸质书id */
    , core.paper_book_product_id AS paper_book_product_id
    /* 对应电子书id */
    , core.ebook_product_id AS ebook_product_id
    /* @TODO 不详 */
    , core.device_type AS device_type
    /* @TODO 不详 */
    , core.device_id AS device_id

    /*================== product_price =======================*/
    /* 折扣 TODO 源数据需改为整型 */
    , price.discount AS discount
    /* 当当卖价*100 */
    , price.dd_sale_price AS dd_sale_price
    /* 原价*100 */
    , price.price AS price
    /* 售价上限*100 */
    , price.sale_price_max AS sale_price_max
    /* 当当卖价上限*100*/
    , price.dd_sale_price_max AS dd_sale_price_max
    
    /*================== b2c_product ========================*/
    /* 描述 */
    , b2c.description AS description
    /* 关键词 */
    , b2c.keyword AS keyword
    /* 属性信息  */
    , b2c.attrib AS attrib
    /* @TODO 不详 */
    , b2c.salespromote AS salespromote
    /* 店铺得分 */
    /*, b2c.shopscore AS shopscore*/
    /* 店铺信息 */
    , b2c.shop_info AS shop_info
    /* 最新上架时间 */
    , b2c.modifytime AS modifytime
    /* 是否是商家自己所卖的商品 */
    , b2c.is_shop_product AS is_shop_product
    /* 是否是公用品(0:否,1:是) */
    , b2c.is_share_product AS is_share_product
    /* 是否是商家商品(0:否,1:是) */
    , b2c.is_shop_sell AS is_shop_sell
    /* 是否是当当在卖(0:否,1:是) */
    , b2c.is_dd_sell AS is_dd_sell
    /* 品牌id */
    , b2c.brand_id AS brand_id
    /* 品牌名 */
    , b2c.brand_name AS brand_name
    
    /*================== product_info  =====================*/
    /* 编辑推荐 */
    , info.abstract AS abstract
    /* 内容摘要 */
    , info.content AS content
    /* 目录 */
    , info.catalog AS catalog
    
    /*================== product_stock  =====================*/
    /* 库存状态(0:不可售1:全国可售2：区域可售) */
    , stock.stock_status AS stock_status
    /* 省市库存状态 */
    , stock.city_stock_status AS city_stock_status

    /*================== search_info  =====================*/
    /* 展示数据 */
    , search_info.isbn_search AS isbn_search
    /* 日销量 */
    , search_info.sale_day AS sale_day
    /* 周销量 */
    , search_info.sale_week AS sale_week
    /* 标题同义词 */
    , search_info.title_synonym AS title_synonym
    /* 标题中心词 */
    , search_info.title_primary AS title_primary
    /* 标题摘要 */
    , search_info.title_sub AS title_sub

    /*==================== category ===================*/
    /* 馆id */
    , category.guan_id AS guan_id

    /*==================== c2c_product ===================*/
    /* 二手书价格范围 */
    , c2c.c2c_second_price AS c2c_second_price
    /* 是否是二手书 */
    , c2c.is_c2c_book AS is_c2c_book
FROM
    prodviewdb.products_core_search core 
    LEFT JOIN
    prodviewdb.product_price price ON 
    core.product_id = price.product_id
    LEFT JOIN
    prodviewdb.b2c_product b2c ON
    core.product_id = b2c.product_id
    LEFT JOIN
    prodviewdb.product_info info ON
    core.product_id = info.product_id
    LEFT JOIN
    prodviewdb.product_stock stock ON
    core.product_id = stock.product_id
    LEFT JOIN
    prodviewdb.search_info search_info ON
    core.product_id = search_info.product_id
    LEFT JOIN
    prodviewdb.set_book set_book ON
    core.product_id = set_book.product_id
    LEFT JOIN
    /* @TODO 性能问题 */
    prodviewdb.category category ON
    core.category_id = category.category_id
    LEFT JOIN
    prodviewdb.c2c_product c2c ON
    core.product_id = c2c.product_id
WHERE
    price.dd_sale_price > 0
    AND (
        stock.stock_status > 0 
        OR core.pre_sale <> 0 
        OR (
            b2c.is_share_product = 1 
            AND b2c.shop_info <> '' 
            AND b2c.shop_info is not null
        )
    ) 
    AND core.product_name <> '' 
    AND core.category_id > 4000000 
    AND (
        (
            (core.is_catalog_product = 0 OR isnull(core.is_catalog_product)) 
            AND (
                core.main_product_id = 0 
                OR isnull(core.main_product_id)
            )
        ) 
        OR core.is_catalog_product = 1
    ) 
    AND (
        core.display_status = 0 
        OR isnull(core.display_status)
    ) 
    AND (
        core.product_type = 0 
        OR core.product_type = 9 
        OR isnull(core.product_type)
    )    
;    

