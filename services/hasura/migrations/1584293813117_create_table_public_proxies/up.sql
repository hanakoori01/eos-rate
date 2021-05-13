
CREATE TABLE "public"."proxies"("owner" text NOT NULL, "name" text NOT NULL, "website" text, "slogan" text, "philosophy" text, "background" text, "logo_256" text, "telegram" text, "steemit" text, "twitter" text, "wechat" text, "voter_info" jsonb NOT NULL, PRIMARY KEY ("owner") , UNIQUE ("owner"), UNIQUE ("name"));