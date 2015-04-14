/*==============================================================*/
/*                   搜索主视图                                 */
/*==============================================================*/
USE prodviewdb;

DROP VIEW IF exists search_v3_view;

/*==============================================================*/
/* VIEW: test_search                                            */
/*==============================================================*/
/* TODO: index view 利用索引视图加速 */
CREATE VIEW search_v3_view AS
SELECT
    /* ================ product_core_search ===================*/
    /* 商品ID 主键 */
    core.product_id AS product_id
    /* 商品名 */
    , core.product_name AS product_name
    /* 商品类型 */
    , core.product_type AS product_type
    /* 商品介质（商品类型标识） */
    , core.product_medium AS product_medium
    /* 分类ID */
    , core.category_id AS category_id
    /* 分类路径（多个） */
    , core.cat_paths AS cat_paths
    /* @SEE 该字段目前为reader_for_search自己生成 */
    /* , core.main_category AS main_category */
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
    /* 图片数 */
    , core.num_images AS num_images
    /* 主产品id */
    , core.main_product_id AS main_product_id
    /* 是否为产品，1是0否 */
    , core.is_catalog_product AS is_catalog_product
    /* 书id? */
    , core.book_id AS book_id
    /* 五星标识 */
    , core.icon_flag_life_five_star AS icon_flag_life_five_star
    /* 独有标识 */
    , core.icon_flag_sole AS icon_flag_sole
    /* 标识新品 */
    , core.icon_flag_mall_new AS icon_flag_mall_new
    /* 标识热卖 */
    , core.icon_flag_mall_hot AS icon_flag_mall_hot
    /* 上榜标识 */
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
    /* @TODO 不详 */
    , core.is_publish AS is_publish
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
    /* 是否有对应电子书 */
    , core.is_has_ebook AS is_has_ebook
    /* 对应纸质书id */
    , core.paper_book_product_id AS paper_book_product_id
    /* 对应电子书id */
    , core.ebook_product_id AS ebook_product_id
    /* @TODO 不详 */
    , core.device_type AS device_type
    /* @TODO 不详 */
    , core.device_id AS device_id
    /* 经销商(CD) */
    , core.distributor AS distributor
    /* 展示数据ISBN */
    , core.standard_id AS standard_id

    /*================== product_price =======================*/
    /* 折扣 TODO 源数据需改为整型 */
    , price.discount AS discount
    /* 当当卖价*100 */
    , price.dd_sale_price AS dd_sale_price
    /* 原价*100 */
    , price.price AS price
    /* 售价上限 */
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
    /* 作者（带格式） */
    , search_info.author_format AS author_format
    /* 演唱者（带格式） */
    , search_info.singer_format AS singer_format
    /* 导演（带格式） */
    , search_info.director_format AS director_format
    /* 日销量 */
    , search_info.sale_day AS sale_day
    /* 周销量 */
    , search_info.sale_week AS sale_week
    /* 月销量 */
    , search_info.sale_month AS sale_month
    /* 日销售额 */
    , search_info.sale_day_amt AS sale_day_amt
    /* 周销售额 */
    , search_info.sale_week_amt AS sale_week_amt
    /* 月销售额 */
    , search_info.sale_month_amt AS sale_month_amt
    /* 标题同义词 */
    , search_info.title_synonym AS title_synonym
    /* 标题中心词 */
    , search_info.title_primary AS title_primary
    /* 标题摘要 */
    , search_info.title_sub AS title_sub

    /*==================== c2c_product ===================*/
    /* 二手书价格范围的下限 */
    , c2c.c2c_price0 AS c2c_price0
    /* 二手书价格范围的上限 */
    , c2c.c2c_price1 AS c2c_price1
    /* 是否是二手书 */
    , c2c.is_c2c_book AS is_c2c_book

    /*================== product_cat_promo =================*/
    /* 类别促销类型 */
    , cat_promo.cat_promotion_type AS cat_promotion_type
    /* 类比促销开始时间 */
    , cat_promo.cat_promo_start_date AS cat_promo_start_date
    /* 类比促销结束时间 */
    , cat_promo.cat_promo_end_date AS cat_promo_end_date
FROM
    prodviewdb.products_core_search core 
    LEFT JOIN
    prodviewdb.b2c_product b2c ON
    core.product_id = b2c.product_id
    LEFT JOIN
    prodviewdb.product_info info ON
    core.product_id = info.product_id
    LEFT JOIN
    prodviewdb.search_info search_info ON
    core.product_id = search_info.product_id
    LEFT JOIN
    prodviewdb.product_price price ON 
    core.product_id = price.product_id
    LEFT JOIN
    prodviewdb.product_stock stock ON
    core.product_id = stock.product_id
    LEFT JOIN
    prodviewdb.c2c_product c2c ON
    core.product_id = c2c.product_id
    LEFT JOIN
    prodviewdb.product_cat_promo cat_promo ON
    core.product_id = cat_promo.product_id
WHERE
    /* 商品名不为空 b2c & pub */
    core.product_name <> ''
    /* 上架状态 b2c & pub */
    AND (
        /* 可以出售正常商品 */ 
        ( core.display_status =0 OR isnull(core.display_status) )
        /* 电子书不考虑1（垃圾商品） */
        OR ( core.product_medium = 22 AND core.display_status <> 1 )
    )
    /* 商品类型 b2c & pub */
    AND (
        ( isnull(core.product_type) OR core.product_type = '' )
        OR ( core.is_publication = 0 
                AND ( core.product_type = 0 OR core.product_type = 9 )
        )           
        /* 商品类型不为赠品 */
        OR ( core.is_publication = 1
                AND ( core.product_type <> 99 AND core.product_type <> 1)
        )
    )
    /* 是否产品 b2c */
    AND (
        core.is_publication = 1
        OR (
            core.is_catalog_product = 1
            OR (
                ( core.is_catalog_product = 0 
                    OR isnull(core.is_catalog_product) 
                )
                AND ( 
                    core.main_product_id = 0 OR isnull(core.main_product_id) 
                )
            )
        )
    )
    /* 售价大于0 b2c */
    AND ( core.is_publication = 1 OR price.dd_sale_price > 0 )
    /* 百货库存不为0 b2c */
    AND (
        core.is_publication = 1
        OR (
            /* 库存不为空（0） */
            stock.stock_status > 0
            /* 预售不为0 */
            OR core.pre_sale <> 0
            OR (
                /* 是公用品且其shop_info不为空 */
                b2c.is_share_product = 1
                AND ( b2c.shop_info is not null AND b2c.shop_info <> '' )
            )
        )
    )
    /* 百货的category_id范围，补差价商品不得入库 b2c */
    AND ( core.is_publication = 1 
        OR (
            core.category_id > 4000000 AND core.category_id <> 4003973
        )
    )
    /* ？出版物为当当图书或者供应商商品 pub @TODO 确定此条件的确切意思 */
    AND ( 
        core.is_publication = 0
        OR (
            ( isnull(core.shop_id) OR core.shop_id = 0 )
            OR ( core.shop_id > 0 AND core.is_publish = 1 )
        )
    )
    /* 出版物介质不能为服装 pub @TODO 确定此判断是否多余 */
    AND ( core.is_publication = 0 OR core.product_medium <> '21' ) 
;    

