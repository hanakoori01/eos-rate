#include <rateproducer.hpp>


ACTION rateproducer::rate(
    name user, 
    name bp, 
    int8_t transparency,
    int8_t infrastructure,
    int8_t trustiness,
    int8_t community,
    int8_t development) {
    check( (transparency+infrastructure+trustiness+community+development), "Error vote must have value for at least one category");  
    check( (MINVAL<= transparency &&  transparency<=MAXVAL ), "Error transparency value out of range");
    check( (MINVAL<= infrastructure &&  infrastructure<=MAXVAL ), "Error infrastructure value out of range" );
    check( (MINVAL<= trustiness &&  trustiness<=MAXVAL ), "Error trustiness value out of range" );
    check( (MINVAL<= development &&  development <=MAXVAL ), "Error development value out of range" );
    check( (MINVAL<= community &&  community<=MAXVAL ), "Error community value out of range" );
    
    //checks if the bp is active 
    check( is_blockproducer(bp), "votes are allowed only for registered block producers" );
    
    eosio::name proxy_name = get_proxy(user);
    if(proxy_name.length()) {
        //active proxy??
        check(is_active_proxy(proxy_name), "votes are allowed only for active proxies" );
        //account votes through a proxy
        check(!(MIN_VOTERS > get_voters(proxy_name)), "delegated proxy does not have enough voters" );
    } else {
        // acount must vote for at least 21 bp
        check(!(MIN_VOTERS > get_voters(user)), "account does not have enough voters" );
    }
        
    // upsert bp rating
    _ratings bps(_self, _self.value);
    auto uniq_rating = (static_cast<uint128_t>(user.value) << 64) | bp.value;

    auto uniq_rating_index = bps.get_index<name("uniqrating")>();
    auto existing_rating = uniq_rating_index.find(uniq_rating);

    if( existing_rating == uniq_rating_index.end() ) {
        bps.emplace(user, [&]( auto& row ) {
            row.id = bps.available_primary_key();
            row.uniq_rating = uniq_rating;
            row.user = user;
            row.bp = bp;
            row.transparency = transparency;
            row.infrastructure = infrastructure;
            row.trustiness = trustiness;
            row.community = community;
            row.development = development ;   
        });
        //save stats
        save_bp_stats(user,
                    bp,
                    transparency,
                    infrastructure,
                    trustiness,
                    community,
                    development);
    

    } else {
        //the voter update its vote
        uniq_rating_index.modify(existing_rating, user, [&]( auto& row ) {
            row.user = user;
            row.bp = bp;
            row.transparency = transparency;
            row.infrastructure = infrastructure;
            row.trustiness = trustiness;
            row.community = community;
            row.development = development ;  
        });
        //update bp stats
        float bp_transparency = 0;
        float bp_infrastructure = 0;
        float bp_trustiness = 0;
        float bp_community = 0;
        float bp_development = 0;
        uint32_t  bp_ratings_cntr = 0;
        float  bp_average = 0;
        calculate_bp_stats (bp,
                            &bp_transparency,
                            &bp_infrastructure,
                            &bp_trustiness,
                            &bp_community,
                            &bp_development,
                            &bp_ratings_cntr,
                            &bp_average);
        update_bp_stats (&user,
                        &bp,
                        &bp_transparency,
                        &bp_infrastructure,
                        &bp_trustiness,
                        &bp_community,
                        &bp_development,
                        &bp_ratings_cntr,
                        &bp_average);
    }    
}
    
void rateproducer::save_bp_stats (
    name user,
    name bp_name,
    float transparency,
    float infrastructure,
    float trustiness,
    float community,
    float development) {
    _stats bps_stats(_self, _self.value);
    auto itr = bps_stats.find(bp_name.value);
    float counter =0;
    float sum = 0;
    if(itr == bps_stats.end()) {
    //new entry
        bps_stats.emplace(user, [&]( auto& row ) {
            if (transparency) {
                row.transparency = transparency;
                counter++;
                sum += transparency;
            }

            if (infrastructure) {
                row.infrastructure = infrastructure;
                counter++;
                sum += infrastructure;
            }

            if (trustiness) {
                row.trustiness = trustiness;
                counter++;
                sum += trustiness;
            }

            if (development) {
                row.development = development;
                counter++;
                sum += development;
            }

            if (community) {
                row.community = community;
                counter++;
                sum += community;
            }

            if(counter) {
                row.bp = bp_name;
                row.ratings_cntr = 1;
                row.average =sum/counter;
                
            }
        });
    } else {
        //update the entry
        bps_stats.modify(itr,user, [&]( auto& row ) {
            if (transparency) {
                sum += transparency;
                if(row.transparency) {
                    transparency = (transparency + row.transparency)/2;
                }
                row.transparency = transparency;
                counter++;
            }

            if (infrastructure) {
                sum += infrastructure;
                if(row.infrastructure) {
                    infrastructure = (infrastructure + row.infrastructure)/2;
                }
                row.infrastructure = infrastructure;
                counter++;
            }

            if (trustiness) {
                sum += trustiness;
                if(row.trustiness) {
                    trustiness = (trustiness + row.trustiness) / 2;
                }
                row.trustiness = trustiness;
                counter++;
            }

            if (development) {
                sum += development;
                if(row.development) {
                    development  = (development + row.development) / 2;
                }
                row.development = development;
                counter++;
            }

            if (community) {
                sum += community;
                if(row.community) {
                    community = (community + row.community) / 2;
                }
                row.community = community;
                counter++;
            }

            if(counter) {
                row.ratings_cntr++;
                row.average =( (sum/counter) + row.average ) / 2;
            }
        });
    }
}

void rateproducer::calculate_bp_stats (
    name bp_name,
    float * transparency,
    float * infrastructure,
    float * trustiness,
    float * community,
    float * development,
    uint32_t * ratings_cntr,
    float  * average) {
    
    float category_counter = 0;
    
    float transparency_total  = 0;
    float infrastructure_total = 0;
    float trustiness_total = 0;
    float community_total = 0;
    float development_total = 0;
    
    float transparency_cntr = 0;
    float infrastructure_cntr = 0;
    float trustiness_cntr = 0;
    float community_cntr = 0;
    float development_cntr = 0;
    uint32_t voters_cntr = 0;

    _ratings bps(_self, _self.value);
    auto bps_index = bps.get_index<name("bp")>();
    auto bps_it = bps_index.find(bp_name.value); 
    
    while(bps_it != bps_index.end()) {
        if(bp_name == bps_it->bp) {
            if(bps_it->transparency) {
                transparency_total+=bps_it->transparency;
                transparency_cntr++;
            }

            if(bps_it->infrastructure) {
                infrastructure_total+=bps_it->infrastructure;
                infrastructure_cntr++;
            }

            if(bps_it->trustiness) {
                trustiness_total+=bps_it->trustiness;
                trustiness_cntr++;
            }

            if(bps_it->community) {
                community_total+=bps_it->community;
                community_cntr++;
            }

            if(bps_it->development) {
                development_total+=bps_it->development;
                development_cntr++;
            }
            voters_cntr++;
        }
        bps_it ++;
    }
    
    if(transparency_cntr) {
        *transparency = transparency_total/transparency_cntr;
        category_counter++;
    }
    
    if(infrastructure_cntr) {
        *infrastructure =infrastructure_total/infrastructure_cntr;
        category_counter++;
    }
        
    if(trustiness_cntr) {
        *trustiness = trustiness_total/trustiness_cntr;
        category_counter++;
    }
        
    if(community_cntr) {
        *community = community_total/community_cntr;
        category_counter++;
    }
        
    if(development_cntr) {
        *development = development_total/development_cntr;
        category_counter++;
    } 
    *average = (*transparency + *infrastructure + *trustiness + *community +*development)/category_counter;
    *ratings_cntr = voters_cntr;
}

void rateproducer::update_bp_stats (
    name * user,
    name * bp_name,
    float * transparency,
    float * infrastructure,
    float * trustiness,
    float * community,
    float * development,
    uint32_t * ratings_cntr,
    float * average) {
    
    _stats bps_stats(_self, _self.value);
    auto itr = bps_stats.find(bp_name->value);
    if(itr != bps_stats.end()) {
        //if rate categories are more than zero
        // we store, otherwise remove the entry
        if( *transparency +
            *infrastructure +
            *trustiness +
            *community +
            *development) {
        
            bps_stats.modify(itr,*user, [&]( auto& row ) {
                row.transparency = *transparency;
                row.infrastructure = *infrastructure;
                row.trustiness = *trustiness;
                row.development = *development;
                row.community = *community;      
                row.ratings_cntr= *ratings_cntr;
                row.average = *average;
                });
        } else {
            bps_stats.erase(itr);
        }  
    }
}

ACTION rateproducer::erase(name bp_name) {
    
    require_auth(_self);

    _ratings bps(_self, _self.value);
    auto itr = bps.begin();
    while (itr != bps.end()) {
        if(itr->bp == bp_name) {
            itr = bps.erase(itr);
        } else {
            itr++;
        }
    }
    
    //clean the stats summary
    _stats bps_stats(_self, _self.value);
    auto itr_stats = bps_stats.find(bp_name.value);
    if (itr_stats != bps_stats.end()) bps_stats.erase(itr_stats);
}

void rateproducer::erase_bp_info(std::set<eosio::name> * bps_to_clean){
    _ratings bps(_self, _self.value);
    _stats bps_stats(_self, _self.value);
    
    std::set<eosio::name>::iterator it;
    for (it = bps_to_clean->begin(); it != bps_to_clean->end(); ++it) {
        //clean all ratings related to an bp
        auto itr = bps.begin();
        while (itr != bps.end()) {
            if(itr->bp == *it) {
                itr = bps.erase(itr);
            } else {
                itr++;
            }
        }
        //clean the stats summary
        auto itr_stats = bps_stats.find((*it).value);
        if (itr_stats != bps_stats.end()) bps_stats.erase(itr_stats);
    }
}

ACTION rateproducer::wipe() {
    
    require_auth(_self);
    _ratings bps(_self, _self.value);
    auto itr = bps.begin();
    while (itr != bps.end()) {
        itr = bps.erase(itr);
    }

    _stats bps_stats(_self, _self.value);
    auto itr_stats = bps_stats.begin();
    while (itr_stats != bps_stats.end()) {
        itr_stats = bps_stats.erase(itr_stats);
    }
}

ACTION rateproducer::rminactive() {
    
    require_auth(_self);
    std::set<eosio::name> noupdated_bps; 
    _stats bps_stats(_self, _self.value);
    auto itr_stats = bps_stats.begin();
    while ( itr_stats != bps_stats.end()) {
        if (!is_blockproducer(itr_stats->bp)) {
            noupdated_bps.insert(itr_stats->bp);
        }
        itr_stats++;
    }
    if(noupdated_bps.size()) erase_bp_info(&noupdated_bps);
    print("bps deleted:",noupdated_bps.size());
}

ACTION rateproducer::rmrate(name user, name bp) {
    require_auth(user);
    
    _ratings bps(_self, _self.value);
    auto uniq_rating = (static_cast<uint128_t>(user.value) << 64) | bp.value;

    auto uniq_rating_index = bps.get_index<name("uniqrating")>();
    auto existing_rating = uniq_rating_index.find(uniq_rating);

    if( existing_rating != uniq_rating_index.end() ) {
        
        //delete rate info
        auto itr = uniq_rating_index.erase(existing_rating);
        
        //update bp stats
        float bp_transparency = 0;
        float bp_infrastructure = 0;
        float bp_trustiness = 0;
        float bp_community = 0;
        float bp_development = 0;
        uint32_t  bp_ratings_cntr = 0;
        float  bp_average = 0;

        //re-calculate stats for the bp 
        calculate_bp_stats (bp,
                            &bp_transparency,
                            &bp_infrastructure,
                            &bp_trustiness,
                            &bp_community,
                            &bp_development,
                            &bp_ratings_cntr,
                            &bp_average);
                            
        //save the re-calcualtes stats
        update_bp_stats (&user,
                        &bp,
                        &bp_transparency,
                        &bp_infrastructure,
                        &bp_trustiness,
                        &bp_community,
                        &bp_development,
                        &bp_ratings_cntr,
                        &bp_average);
            
    }
}