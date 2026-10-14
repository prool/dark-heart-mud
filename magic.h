/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/
 

/*
 * Spell functions.
 * Defined in magic.c.
 */
DECLARE_SPELL_FUN(      spell_null              );
DECLARE_SPELL_FUN(      spell_retribution       );
DECLARE_SPELL_FUN(      spell_acid_blast        );
DECLARE_SPELL_FUN(      spell_instill_life      );
DECLARE_SPELL_FUN(      spell_allhallowseve     );
DECLARE_SPELL_FUN(      spell_storm_herald      );
DECLARE_SPELL_FUN(      spell_rock_to_mud       );
DECLARE_SPELL_FUN(      spell_wither            );
DECLARE_SPELL_FUN(      spell_resist_cold       );
DECLARE_SPELL_FUN(      spell_armor             );
DECLARE_SPELL_FUN(      spell_unravel           );
DECLARE_SPELL_FUN(      spell_evards_tenticles  );
DECLARE_SPELL_FUN(      spell_wyvern_watch      );
DECLARE_SPELL_FUN(      spell_regrowth          );
DECLARE_SPELL_FUN(      spell_clfe              );
DECLARE_SPELL_FUN(      spell_haven	        );
DECLARE_SPELL_FUN(      spell_ghost_light       );
DECLARE_SPELL_FUN(      spell_raise_dead        );
DECLARE_SPELL_FUN(      spell_clwe              );
DECLARE_SPELL_FUN(      spell_pestilence        );
DECLARE_SPELL_FUN(      spell_consecrate        );
DECLARE_SPELL_FUN(      spell_clae              );
DECLARE_SPELL_FUN(      spell_clee              );
DECLARE_SPELL_FUN(      spell_cgfe              );
DECLARE_SPELL_FUN(      spell_cgwe              );
DECLARE_SPELL_FUN(      spell_cgae              );
DECLARE_SPELL_FUN(      spell_cgee              );
DECLARE_SPELL_FUN(      spell_arachnophobia     );
DECLARE_SPELL_FUN(      spell_illumination      );
DECLARE_SPELL_FUN(      spell_blood_spew        );
DECLARE_SPELL_FUN(      spell_disintegrate      );
DECLARE_SPELL_FUN(      spell_mummy_rot         );
DECLARE_SPELL_FUN(      spell_time_sight        );
DECLARE_SPELL_FUN(      spell_chelchemy         );
DECLARE_SPELL_FUN(      spell_vivify            );
DECLARE_SPELL_FUN(      spell_imbue	        );
DECLARE_SPELL_FUN(      spell_ballistic_attack  );
DECLARE_SPELL_FUN(      spell_bless             );
DECLARE_SPELL_FUN(      spell_bind_daemon       );
DECLARE_SPELL_FUN(      spell_grounding         );
DECLARE_SPELL_FUN(      spell_blindness         );
DECLARE_SPELL_FUN(      spell_bloodlust         );
DECLARE_SPELL_FUN(      spell_hideous_laughter  );
DECLARE_SPELL_FUN(      spell_burning_hands     );
DECLARE_SPELL_FUN(      spell_reflection        );
DECLARE_SPELL_FUN(      spell_call_lightning    );
DECLARE_SPELL_FUN(      spell_calm              );
DECLARE_SPELL_FUN(      spell_animate_dead      );
DECLARE_SPELL_FUN(      spell_dark_pact         );
DECLARE_SPELL_FUN(      spell_ged_transform     );
DECLARE_SPELL_FUN(      spell_air_barrier       );
DECLARE_SPELL_FUN(      spell_cancellation      );
DECLARE_SPELL_FUN(      spell_improved_slow     );
DECLARE_SPELL_FUN(      spell_improved_haste    );
DECLARE_SPELL_FUN(      spell_cause_critical    );
DECLARE_SPELL_FUN(      spell_deafness          );
DECLARE_SPELL_FUN(      spell_golum 	        );
DECLARE_SPELL_FUN(      spell_cause_light       );
DECLARE_SPELL_FUN(      spell_tree		);
DECLARE_SPELL_FUN(      spell_accuracy          );
DECLARE_SPELL_FUN(      spell_regeneration      );
DECLARE_SPELL_FUN(      spell_jump              );
DECLARE_SPELL_FUN(      spell_cause_serious     );
DECLARE_SPELL_FUN(      spell_change_sex        );
DECLARE_SPELL_FUN(      spell_chain_lightning   );
DECLARE_SPELL_FUN(      spell_charm_person      );
DECLARE_SPELL_FUN(      spell_whispers 	        );
DECLARE_SPELL_FUN(      spell_desecrate_exit    );
DECLARE_SPELL_FUN(      spell_chill_touch       );
DECLARE_SPELL_FUN(      spell_colour_spray      );
DECLARE_SPELL_FUN(      spell_continual_light   );
DECLARE_SPELL_FUN(      spell_cockroach         );
DECLARE_SPELL_FUN(      spell_conflaguration    );
DECLARE_SPELL_FUN(      spell_control_weather   );
DECLARE_SPELL_FUN(      spell_consultation      );
DECLARE_SPELL_FUN(      spell_create_gourd      );
DECLARE_SPELL_FUN(      spell_create_food       );
DECLARE_SPELL_FUN(      spell_create_rose       );
DECLARE_SPELL_FUN(      spell_create_spring     );
DECLARE_SPELL_FUN(      spell_create_water      );
DECLARE_SPELL_FUN(      spell_cry_of_anger      );
DECLARE_SPELL_FUN(      spell_cure_blindness    );
DECLARE_SPELL_FUN(      spell_cure_critical     );
DECLARE_SPELL_FUN(      spell_cure_disease      );
DECLARE_SPELL_FUN(      spell_cure_light        );
DECLARE_SPELL_FUN(      spell_cure_poison       );
DECLARE_SPELL_FUN(      spell_cure_serious      );
DECLARE_SPELL_FUN(      spell_curse             );
DECLARE_SPELL_FUN(      spell_misfortune        );
DECLARE_SPELL_FUN(      spell_shrink            );
DECLARE_SPELL_FUN(      spell_growth            );
DECLARE_SPELL_FUN(      spell_nystul            );
DECLARE_SPELL_FUN(      spell_obscure_magic     );
DECLARE_SPELL_FUN(      spell_waterfall         );
DECLARE_SPELL_FUN(      spell_demonfire         );
DECLARE_SPELL_FUN(      spell_wall_of_iron      );
DECLARE_SPELL_FUN(      spell_detect_evil       );
DECLARE_SPELL_FUN(      spell_detect_good       );
DECLARE_SPELL_FUN(      spell_detect_hidden     );
DECLARE_SPELL_FUN(      spell_detect_invis      );
DECLARE_SPELL_FUN(      spell_detect_magic      );
DECLARE_SPELL_FUN(      spell_stinking_cloud    );
DECLARE_SPELL_FUN(      spell_life_vortex       );
DECLARE_SPELL_FUN(      spell_death_vortex      );
DECLARE_SPELL_FUN(      spell_detect_poison     );
DECLARE_SPELL_FUN(      spell_wall_of_fire      );
DECLARE_SPELL_FUN(      spell_devour_soul       );
DECLARE_SPELL_FUN(      spell_dispel_evil       );
DECLARE_SPELL_FUN(      spell_dispel_good       );
DECLARE_SPELL_FUN(      spell_dispel_magic      );
DECLARE_SPELL_FUN(      spell_dragon_trans      );
DECLARE_SPELL_FUN(      spell_earthquake        );
DECLARE_SPELL_FUN(      spell_enchant_armor     );
DECLARE_SPELL_FUN(      spell_enchant_weapon    );
DECLARE_SPELL_FUN(      spell_energy_drain      );
DECLARE_SPELL_FUN(      spell_faerie_fire       );
DECLARE_SPELL_FUN(      spell_faerie_fog        );
DECLARE_SPELL_FUN(      spell_farsight          );
DECLARE_SPELL_FUN(      spell_fear              );
DECLARE_SPELL_FUN(      spell_fireball          );
DECLARE_SPELL_FUN(      spell_fireproof         );
DECLARE_SPELL_FUN(      spell_collapse          );
DECLARE_SPELL_FUN(      spell_flame_blade       );
DECLARE_SPELL_FUN(      spell_flamestrike       );
DECLARE_SPELL_FUN(      spell_floating_disc     );
DECLARE_SPELL_FUN(      spell_fly               );
DECLARE_SPELL_FUN(      spell_frenzy            );
DECLARE_SPELL_FUN(      spell_frog              );
DECLARE_SPELL_FUN(      spell_gate              );
DECLARE_SPELL_FUN(      spell_glitterdust       );
DECLARE_SPELL_FUN(      spell_giant_strength    );
DECLARE_SPELL_FUN(      spell_harm              );
DECLARE_SPELL_FUN(      spell_harmony           );
DECLARE_SPELL_FUN(      spell_haste             );
DECLARE_SPELL_FUN(      spell_heal              );
DECLARE_SPELL_FUN(      spell_heat_metal        );
DECLARE_SPELL_FUN(      spell_hold_person       );
DECLARE_SPELL_FUN(      spell_holy_word         );
DECLARE_SPELL_FUN(      spell_horrid_wilting    );
DECLARE_SPELL_FUN(      spell_hunger            );
DECLARE_SPELL_FUN(      spell_identify          );
DECLARE_SPELL_FUN(      spell_infravision       );
DECLARE_SPELL_FUN(      spell_invis             );
DECLARE_SPELL_FUN(      spell_improved_invis    );
DECLARE_SPELL_FUN(      spell_iron_skin         );
DECLARE_SPELL_FUN(      spell_know_alignment    );
DECLARE_SPELL_FUN(      spell_lava_burst        );
DECLARE_SPELL_FUN(      spell_magma_burst       );
DECLARE_SPELL_FUN(      spell_lightning_bolt    );
DECLARE_SPELL_FUN(      spell_locate_object     );
DECLARE_SPELL_FUN(      spell_lure_of_darkness  );
DECLARE_SPELL_FUN(      spell_magic_missile     );
DECLARE_SPELL_FUN(      spell_mass_healing      );
DECLARE_SPELL_FUN(      spell_mass_invis        );
DECLARE_SPELL_FUN(      spell_pass_door         );
DECLARE_SPELL_FUN(      spell_plague            );
DECLARE_SPELL_FUN(      spell_poison            );
DECLARE_SPELL_FUN(      spell_pentagram         );
DECLARE_SPELL_FUN(      spell_portal            );
DECLARE_SPELL_FUN(      spell_power_word_kill   );
DECLARE_SPELL_FUN(      spell_power_word_stun   );
DECLARE_SPELL_FUN(      spell_protection_evil   );
DECLARE_SPELL_FUN(      spell_protection_good   );
DECLARE_SPELL_FUN(      spell_quench            );
DECLARE_SPELL_FUN(      spell_ray_of_truth      );
DECLARE_SPELL_FUN(      spell_recharge          );
DECLARE_SPELL_FUN(      spell_refresh           );
DECLARE_SPELL_FUN(      spell_remove_fear       );
DECLARE_SPELL_FUN(      spell_reset             );
DECLARE_SPELL_FUN(      spell_remove_curse      );
DECLARE_SPELL_FUN(      spell_sanctuary         );
DECLARE_SPELL_FUN(      spell_shadow_blade      );
DECLARE_SPELL_FUN(      spell_shocking_grasp    );
DECLARE_SPELL_FUN(      spell_shield            );
DECLARE_SPELL_FUN(      spell_sleep             );
DECLARE_SPELL_FUN(      spell_slow              );
DECLARE_SPELL_FUN(      spell_soul_blade        );
DECLARE_SPELL_FUN(      spell_spring_of_life    );
DECLARE_SPELL_FUN(      spell_star_storm        );
DECLARE_SPELL_FUN(      spell_sunbeam           );
DECLARE_SPELL_FUN(      spell_stone_grip        );
DECLARE_SPELL_FUN(      spell_stone_skin        );
DECLARE_SPELL_FUN(      spell_summon            );
DECLARE_SPELL_FUN(      spell_teleport          );
DECLARE_SPELL_FUN(      spell_time_twister      );
DECLARE_SPELL_FUN(      spell_thirst            );
DECLARE_SPELL_FUN(      spell_trip              );
DECLARE_SPELL_FUN(      spell_ventriloquate     );
DECLARE_SPELL_FUN(      spell_phant_force       );
DECLARE_SPELL_FUN(      spell_imp_phant_force   );
DECLARE_SPELL_FUN(      spell_water_breathing   );
DECLARE_SPELL_FUN(      spell_weaken            );
DECLARE_SPELL_FUN(      spell_windtomb          );
DECLARE_SPELL_FUN(      spell_wish              );
DECLARE_SPELL_FUN(      spell_heroes_feast      );
DECLARE_SPELL_FUN(      spell_winds_caina       );
DECLARE_SPELL_FUN(      spell_word_of_recall    );
DECLARE_SPELL_FUN(      spell_general_purpose   );
DECLARE_SPELL_FUN(      spell_high_explosive    );
DECLARE_SPELL_FUN(      spell_iceball	        );

