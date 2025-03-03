#include <Client/BuzzHouse/Generator/SQLCatalog.h>
#include <Client/BuzzHouse/Generator/StatementGenerator.h>

namespace BuzzHouse
{

void StatementGenerator::prepareNextExplain(RandomGenerator & rg, ExplainQuery * eq)
{
    std::unordered_map<uint32_t, QueryLevel> levels_backup;
    std::vector<uint32_t> ids_backup;
    std::vector<ColumnPathChain> entries_backup;
    std::vector<ColumnPathChain> table_entries_backup;
    std::vector<ColumnPathChain> remote_entries_backup;
    const bool prev_in_transaction = this->in_transaction;
    const bool prev_inside_projection = this->inside_projection;
    const bool prev_allow_not_deterministic = this->allow_not_deterministic;
    const bool prev_allow_in_expression_alias = this->allow_in_expression_alias;
    const bool prev_allow_subqueries = this->allow_subqueries;
    const bool prev_enforce_final = this->enforce_final;
    const bool prev_allow_engine_udf = this->allow_engine_udf;

    /// Backup everything
    for (const auto & entry : this->levels)
    {
        levels_backup[entry.first] = entry.second;
    }
    this->levels.clear();
    ids_backup.reserve(this->ids.size());
    for (const auto & entry : this->ids)
    {
        ids_backup.emplace_back(entry);
    }
    this->ids.clear();
    entries_backup.reserve(this->entries.size());
    for (const auto & entry : this->entries)
    {
        entries_backup.emplace_back(entry);
    }
    this->entries.clear();
    table_entries_backup.reserve(this->table_entries.size());
    for (const auto & entry : this->table_entries)
    {
        table_entries_backup.emplace_back(entry);
    }
    this->table_entries.clear();
    remote_entries_backup.reserve(this->remote_entries.size());
    for (const auto & entry : this->remote_entries)
    {
        remote_entries_backup.emplace_back(entry);
    }
    this->remote_entries.clear();

    generateNextExplain(rg, eq);

    this->levels.clear();
    for (const auto & entry : levels_backup)
    {
        this->levels[entry.first] = entry.second;
    }
    this->ids.clear();
    this->ids.reserve(ids_backup.size());
    for (const auto & entry : ids_backup)
    {
        this->ids.emplace_back(entry);
    }
    this->entries.clear();
    this->entries.reserve(entries_backup.size());
    for (const auto & entry : entries_backup)
    {
        this->entries.emplace_back(entry);
    }
    this->table_entries.clear();
    this->table_entries.reserve(table_entries_backup.size());
    for (const auto & entry : table_entries_backup)
    {
        this->table_entries.emplace_back(entry);
    }
    this->remote_entries.clear();
    this->remote_entries.reserve(remote_entries_backup.size());
    for (const auto & entry : remote_entries_backup)
    {
        this->remote_entries.emplace_back(entry);
    }
    this->in_transaction = prev_in_transaction;
    this->inside_projection = prev_inside_projection;
    this->allow_not_deterministic = prev_allow_not_deterministic;
    this->allow_in_expression_alias = prev_allow_in_expression_alias;
    this->allow_subqueries = prev_allow_subqueries;
    this->enforce_final = prev_enforce_final;
    this->allow_engine_udf = prev_allow_engine_udf;

    /// Don't let superfluous entries stay
    this->staged_databases.clear();
    this->staged_tables.clear();
    this->staged_views.clear();
    this->staged_functions.clear();
}

void StatementGenerator::generateArrayJoin(RandomGenerator & rg, ArrayJoin * aj)
{
    SQLRelation rel("");
    std::vector<SQLRelationCol> available_cols;

    if (!this->levels[this->current_level].rels.empty())
    {
        for (const auto & entry : this->levels[this->current_level].rels)
        {
            available_cols.insert(available_cols.end(), entry.cols.begin(), entry.cols.end());
        }
        std::shuffle(available_cols.begin(), available_cols.end(), rg.generator);
    }
    aj->set_left(rg.nextBool());
    const uint32_t nccols = std::min<uint32_t>(
        UINT32_C(3), (rg.nextRandomUInt32() % (available_cols.empty() ? 3 : static_cast<uint32_t>(available_cols.size()))) + 1);
    const uint32_t nclauses = std::min<uint32_t>(this->fc.max_width - this->width, nccols);

    for (uint32_t i = 0; i < nclauses; i++)
    {
        const String cname = "c" + std::to_string(this->levels[this->current_level].aliases_counter++);
        ExprColAlias * eca = i == 0 ? aj->mutable_constraint() : aj->add_other_constraints();
        Expr * expr = eca->mutable_expr();

        if (!available_cols.empty() && rg.nextSmallNumber() < 8)
        {
            const SQLRelationCol & rel_col = available_cols[i];
            ExprSchemaTableColumn * estc = expr->mutable_comp_expr()->mutable_expr_stc();
            ExprColumn * ecol = estc->mutable_col();

            if (!rel_col.rel_name.empty())
            {
                estc->mutable_table()->set_table(rel_col.rel_name);
            }
            rel_col.AddRef(ecol);
            addFieldAccess(rg, expr, 6);
            addColNestedAccess(rg, ecol, 6);
        }
        else
        {
            generateExpression(rg, expr);
        }
        rel.cols.emplace_back(SQLRelationCol("", {cname}));
        eca->mutable_col_alias()->set_column(cname);
    }
    this->levels[this->current_level].rels.emplace_back(rel);
}

void StatementGenerator::generateDerivedTable(
    RandomGenerator & rg, SQLRelation & rel, const uint32_t allowed_clauses, const uint32_t ncols, Select * sel)
{
    std::unordered_map<uint32_t, QueryLevel> levels_backup;

    for (const auto & entry : this->levels)
    {
        levels_backup[entry.first] = entry.second;
    }
    this->levels.clear();

    this->current_level++;
    this->levels[this->current_level] = QueryLevel(this->current_level);
    generateSelect(rg, false, false, ncols, allowed_clauses, sel);
    this->current_level--;

    for (const auto & entry : levels_backup)
    {
        this->levels[entry.first] = entry.second;
    }

    if (sel->has_select_core())
    {
        const SelectStatementCore & scc = sel->select_core();

        for (int i = 0; i < scc.result_columns_size(); i++)
        {
            rel.cols.emplace_back(SQLRelationCol(rel.name, {scc.result_columns(i).eca().col_alias().column()}));
        }
    }
    else if (sel->has_set_query())
    {
        const ExplainQuery * aux = &sel->set_query().sel1();

        while (!aux->is_explain() && aux->inner_query().select().sel().has_set_query())
        {
            aux = &aux->inner_query().select().sel().set_query().sel1();
        }

        if (aux->is_explain())
        {
            rel.cols.emplace_back(SQLRelationCol(rel.name, {"explain"}));
        }
        else if (aux->inner_query().select().sel().has_select_core())
        {
            const SelectStatementCore & scc = aux->inner_query().select().sel().select_core();

            for (int i = 0; i < scc.result_columns_size(); i++)
            {
                rel.cols.emplace_back(SQLRelationCol(rel.name, {scc.result_columns(i).eca().col_alias().column()}));
            }
        }
    }
    if (rel.cols.empty())
    {
        rel.cols.emplace_back(SQLRelationCol(rel.name, {"c0"}));
    }
}

void StatementGenerator::setTableRemote(RandomGenerator & rg, const bool table_engine, const SQLTable & t, TableFunction * tfunc)
{
    if (!table_engine && t.hasClickHousePeer())
    {
        const ServerCredentials & sc = fc.clickhouse_server.value();
        RemoteFunc * rfunc = tfunc->mutable_remote();
        ExprSchemaTable * est = rfunc->mutable_tof()->mutable_est();

        rfunc->set_address(sc.hostname + ":" + std::to_string(sc.port));
        est->mutable_database()->set_database("d" + (t.db ? std::to_string(t.db->dname) : "efault"));
        est->mutable_table()->set_table("t" + std::to_string(t.tname));
        rfunc->set_user(sc.user);
        rfunc->set_password(sc.password);
    }
    else if ((table_engine && t.isMySQLEngine() && rg.nextSmallNumber() < 7) || (!table_engine && t.hasMySQLPeer()))
    {
        const ServerCredentials & sc = fc.mysql_server.value();
        MySQLFunc * mfunc = tfunc->mutable_mysql();

        mfunc->set_address(sc.hostname + ":" + std::to_string(sc.mysql_port ? sc.mysql_port : sc.port));
        mfunc->set_rdatabase(sc.database);
        mfunc->set_rtable("t" + std::to_string(t.tname));
        mfunc->set_user(sc.user);
        mfunc->set_password(sc.password);
    }
    else if ((table_engine && t.isPostgreSQLEngine() && rg.nextSmallNumber() < 7) || (!table_engine && t.hasPostgreSQLPeer()))
    {
        const ServerCredentials & sc = fc.postgresql_server.value();
        PostgreSQLFunc * pfunc = tfunc->mutable_postgresql();

        pfunc->set_address(sc.hostname + ":" + std::to_string(sc.port));
        pfunc->set_rdatabase(sc.database);
        pfunc->set_rtable("t" + std::to_string(t.tname));
        pfunc->set_user(sc.user);
        pfunc->set_password(sc.password);
        pfunc->set_rschema("test");
    }
    else if ((table_engine && t.isSQLiteEngine() && rg.nextSmallNumber() < 7) || (!table_engine && t.hasSQLitePeer()))
    {
        SQLiteFunc * sfunc = tfunc->mutable_sqite();

        sfunc->set_rdatabase(connections.getSQLitePath().generic_string());
        sfunc->set_rtable("t" + std::to_string(t.tname));
    }
    else if (table_engine && t.isS3Engine() && rg.nextSmallNumber() < 7)
    {
        String buf;
        bool first = true;
        const ServerCredentials & sc = fc.minio_server.value();
        S3Func * sfunc = tfunc->mutable_s3();

        sfunc->set_resource(
            "http://" + sc.hostname + ":" + std::to_string(sc.port) + sc.database + "/file" + std::to_string(t.tname)
            + (t.isS3QueueEngine() ? "/" : "") + (rg.nextBool() ? "*" : ""));
        sfunc->set_user(sc.user);
        sfunc->set_password(sc.password);
        sfunc->set_format(t.file_format);
        flatTableColumnPath(to_remote_entries, t, [](const SQLColumn &) { return true; });
        for (const auto & entry : this->remote_entries)
        {
            SQLType * tp = entry.getBottomType();

            buf += fmt::format(
                "{}{} {}{}",
                first ? "" : ", ",
                entry.getBottomName(),
                tp->typeName(true),
                entry.nullable.has_value() ? (entry.nullable.value() ? " NULL" : " NOT NULL") : "");
            first = false;
        }
        this->remote_entries.clear();
        sfunc->set_structure(buf);
        if (!t.file_comp.empty())
        {
            sfunc->set_fcomp(t.file_comp);
        }
    }
    else
    {
        RemoteFunc * rfunc = tfunc->mutable_remote();

        chassert(table_engine);
        rfunc->set_address(fc.getConnectionHostAndPort());
        ExprSchemaTable * est = rfunc->mutable_tof()->mutable_est();

        est->mutable_database()->set_database("d" + (t.db ? std::to_string(t.db->dname) : "efault"));
        est->mutable_table()->set_table("t" + std::to_string(t.tname));
    }
}

bool StatementGenerator::joinedTableOrFunction(
    RandomGenerator & rg, const String & rel_name, const uint32_t allowed_clauses, const bool under_remote, TableOrFunction * tof)
{
    bool set_final = false;
    const auto has_table_lambda = [&](const SQLTable & tt)
    {
        return tt.isAttached()
            /* When comparing query success results, don't use tables from other RDBMS, SQL is very undefined */
            && (this->allow_engine_udf || !tt.isAnotherRelationalDatabaseEngine())
            /* When a query is going to be compared against another ClickHouse server, make sure all tables exist in that server */
            && (this->peer_query != PeerQuery::ClickHouseOnly || tt.hasClickHousePeer())
            /* Don't use tables backing not deterministic views in query oracles */
            && (tt.is_deterministic || this->allow_not_deterministic);
    };
    const auto has_view_lambda
        = [&](const SQLView & vv) { return vv.isAttached() && (vv.is_deterministic || this->allow_not_deterministic); };
    const bool has_table = collectionHas<SQLTable>(has_table_lambda);
    const bool has_view = collectionHas<SQLView>(has_view_lambda);
    const bool can_recurse = this->depth < this->fc.max_depth && this->width < this->fc.max_width;

    const uint32_t derived_table = 30 * static_cast<uint32_t>(can_recurse);
    const uint32_t cte = 10 * static_cast<uint32_t>(!under_remote && !this->ctes.empty());
    const uint32_t table = (40 * static_cast<uint32_t>(has_table)) + (20 * static_cast<uint32_t>(this->peer_query != PeerQuery::None));
    const uint32_t view = 20 * static_cast<uint32_t>(this->peer_query != PeerQuery::ClickHouseOnly && has_view);
    const uint32_t engine_udf = 5 * static_cast<uint32_t>(this->allow_engine_udf && (can_recurse || has_table || has_view));
    const uint32_t generate_series_udf = 10;
    const uint32_t system_table = 3 * static_cast<uint32_t>(this->allow_not_deterministic && !systemTables.empty());
    const uint32_t merge_udf = 2 * static_cast<uint32_t>(this->allow_engine_udf);
    const uint32_t cluster_udf
        = 5 * static_cast<uint32_t>(!fc.clusters.empty() && this->allow_engine_udf && (can_recurse || has_table || has_view));
    const uint32_t merge_index_udf = 3 * static_cast<uint32_t>(has_table && this->allow_engine_udf);
    const uint32_t loop_udf = 3 * static_cast<uint32_t>(fc.allow_infinite_tables && this->allow_engine_udf && can_recurse);
    const uint32_t prob_space = derived_table + cte + table + view + engine_udf + generate_series_udf + system_table + merge_udf
        + cluster_udf + merge_index_udf + loop_udf;
    std::uniform_int_distribution<uint32_t> next_dist(1, prob_space);
    const uint32_t nopt = next_dist(rg.generator);

    if (derived_table && (nopt < derived_table + 1))
    {
        /// A derived query
        SQLRelation rel(rel_name);
        ExplainQuery * eq = tof->mutable_select();
        const uint32_t ncols = std::min(this->fc.max_width - this->width, (rg.nextMediumNumber() % UINT32_C(5)) + 1);

        if (ncols == 1 && rg.nextMediumNumber() < 6)
        {
            prepareNextExplain(rg, eq);
            rel.cols.emplace_back(SQLRelationCol(rel_name, {"explain"}));
        }
        else
        {
            generateDerivedTable(rg, rel, allowed_clauses, ncols, eq->mutable_inner_query()->mutable_select()->mutable_sel());
        }
        this->levels[this->current_level].rels.emplace_back(rel);
    }
    else if (cte && nopt < (derived_table + cte + 1))
    {
        SQLRelation rel(rel_name);
        const auto & next_cte = rg.pickValueRandomlyFromMap(rg.pickValueRandomlyFromMap(this->ctes));

        tof->mutable_est()->mutable_table()->set_table(next_cte.name);
        for (const auto & entry : next_cte.cols)
        {
            rel.cols.push_back(entry);
        }
        this->levels[this->current_level].rels.emplace_back(rel);
    }
    else if (table && nopt < (derived_table + cte + table + 1))
    {
        ExprSchemaTable * est = tof->mutable_est();
        const SQLTable & t = rg.pickRandomly(filterCollection<SQLTable>(has_table_lambda));

        if (t.db)
        {
            est->mutable_database()->set_database("d" + std::to_string(t.db->dname));
        }
        est->mutable_table()->set_table("t" + std::to_string(t.tname));
        set_final = t.supportsFinal() && (this->enforce_final || rg.nextSmallNumber() < 3);
        addTableRelation(rg, true, rel_name, t);
    }
    else if (view && nopt < (derived_table + cte + table + view + 1))
    {
        ExprSchemaTable * est = tof->mutable_est();
        const SQLView & v = rg.pickRandomly(filterCollection<SQLView>(has_view_lambda));

        if (v.db)
        {
            est->mutable_database()->set_database("d" + std::to_string(v.db->dname));
        }
        est->mutable_table()->set_table("v" + std::to_string(v.tname));
        set_final = !v.is_materialized && (this->enforce_final || rg.nextSmallNumber() < 3);
        addViewRelation(rel_name, v);
    }
    else if (engine_udf && nopt < (derived_table + cte + table + view + engine_udf + 1))
    {
        const uint32_t remote_table = 10 * static_cast<uint32_t>(has_table);
        const uint32_t remote_view = 5 * static_cast<uint32_t>(has_view);
        const uint32_t recurse = 10 * static_cast<uint32_t>(can_recurse);
        const uint32_t pspace = remote_table + remote_view + recurse;
        std::uniform_int_distribution<uint32_t> ndist(1, pspace);
        const uint32_t nopt2 = ndist(rg.generator);

        if (remote_table && nopt2 < (remote_table + 1))
        {
            const SQLTable & t = rg.pickRandomly(filterCollection<SQLTable>(has_table_lambda));

            setTableRemote(rg, true, t, tof->mutable_tfunc());
            addTableRelation(rg, true, rel_name, t);
        }
        else if (remote_view && nopt2 < (remote_table + remote_view + 1))
        {
            RemoteFunc * rfunc = tof->mutable_tfunc()->mutable_remote();
            ExprSchemaTable * est = rfunc->mutable_tof()->mutable_est();
            const SQLView & v = rg.pickRandomly(filterCollection<SQLView>(has_view_lambda));

            rfunc->set_address(fc.getConnectionHostAndPort());
            est->mutable_database()->set_database("d" + (v.db ? std::to_string(v.db->dname) : "efault"));
            est->mutable_table()->set_table("v" + std::to_string(v.tname));
            addViewRelation(rel_name, v);
        }
        else if (recurse && nopt2 < (remote_table + remote_view + recurse + 1))
        {
            /// This is the tricky part
            RemoteFunc * rfunc = tof->mutable_tfunc()->mutable_remote();

            rfunc->set_address(fc.getConnectionHostAndPort());
            /// Here don't care about the returned result
            this->depth++;
            auto u = joinedTableOrFunction(rg, rel_name, allowed_clauses, true, rfunc->mutable_tof());
            UNUSED(u);
            this->depth--;
        }
        else
        {
            chassert(0);
        }
    }
    else if (generate_series_udf && nopt < (derived_table + cte + table + view + engine_udf + generate_series_udf + 1))
    {
        SQLRelation rel(rel_name);
        std::unordered_map<uint32_t, QueryLevel> levels_backup;
        const uint32_t noption = rg.nextSmallNumber();
        Expr * limit = nullptr;
        TableFunction * tf = tof->mutable_tfunc();
        GenerateSeriesFunc * gsf = tf->mutable_gseries();
        const GenerateSeriesFunc_GSName val = static_cast<GenerateSeriesFunc_GSName>(
            (rg.nextRandomUInt32() % static_cast<uint32_t>(GenerateSeriesFunc_GSName_GSName_MAX)) + 1);
        const String & cname = val == GenerateSeriesFunc_GSName::GenerateSeriesFunc_GSName_numbers ? "number" : "generate_series";

        gsf->set_fname(val);
        for (const auto & entry : this->levels)
        {
            levels_backup[entry.first] = entry.second;
        }
        this->levels.clear();
        if (val == GenerateSeriesFunc_GSName::GenerateSeriesFunc_GSName_numbers)
        {
            if (noption < 4)
            {
                /// 1 arg
                limit = gsf->mutable_expr1();
            }
            else
            {
                /// 2 args
                if (rg.nextBool())
                {
                    gsf->mutable_expr1()->mutable_lit_val()->mutable_int_lit()->set_uint_lit(rg.nextRandomUInt64() % 10000);
                }
                else
                {
                    generateExpression(rg, gsf->mutable_expr1());
                }
                limit = gsf->mutable_expr2();
                if (noption >= 8)
                {
                    /// 3 args
                    if (rg.nextBool())
                    {
                        gsf->mutable_expr3()->mutable_lit_val()->mutable_int_lit()->set_uint_lit(rg.nextRandomUInt64() % 10000);
                    }
                    else
                    {
                        generateExpression(rg, gsf->mutable_expr3());
                    }
                }
            }
        }
        else
        {
            //2 args
            if (rg.nextBool())
            {
                gsf->mutable_expr1()->mutable_lit_val()->mutable_int_lit()->set_uint_lit(rg.nextRandomUInt64() % 10000);
            }
            else
            {
                generateExpression(rg, gsf->mutable_expr1());
            }
            limit = gsf->mutable_expr2();
            if (noption >= 6)
            {
                //3 args
                if (rg.nextBool())
                {
                    gsf->mutable_expr3()->mutable_lit_val()->mutable_int_lit()->set_uint_lit(rg.nextRandomUInt64() % 10000);
                }
                else
                {
                    generateExpression(rg, gsf->mutable_expr3());
                }
            }
        }
        for (const auto & entry : levels_backup)
        {
            this->levels[entry.first] = entry.second;
        }

        limit->mutable_lit_val()->mutable_int_lit()->set_uint_lit(rg.nextRandomUInt64() % 10000);
        rel.cols.emplace_back(SQLRelationCol(rel_name, {cname}));
        this->levels[this->current_level].rels.emplace_back(rel);
    }
    else if (system_table && nopt < (derived_table + cte + table + view + engine_udf + generate_series_udf + system_table + 1))
    {
        SQLRelation rel(rel_name);
        ExprSchemaTable * est = tof->mutable_est();
        const auto & ntable = rg.pickRandomly(systemTables);
        const auto & tentries = systemTables.at(ntable);

        est->mutable_database()->set_database("system");
        est->mutable_table()->set_table(ntable);
        for (const auto & entry : tentries)
        {
            rel.cols.emplace_back(SQLRelationCol(rel_name, {entry}));
        }
        this->levels[this->current_level].rels.emplace_back(rel);
    }
    else if (merge_udf && nopt < (derived_table + cte + table + view + engine_udf + generate_series_udf + system_table + merge_udf + 1))
    {
        SQLRelation rel(rel_name);
        TableFunction * tf = tof->mutable_tfunc();
        MergeFunc * mdf = tf->mutable_merge();

        if (rg.nextBool())
        {
            mdf->set_mdatabase(setMergeTableParameter<std::shared_ptr<SQLDatabase>>(rg, "d"));
        }
        mdf->set_mtable(rg.nextBool() ? setMergeTableParameter<SQLTable>(rg, "t") : setMergeTableParameter<SQLView>(rg, "v"));
        for (uint32_t i = 0; i < 6; i++)
        {
            rel.cols.emplace_back(SQLRelationCol(rel_name, {"c" + std::to_string(i)}));
        }
        this->levels[this->current_level].rels.emplace_back(rel);
    }
    else if (
        cluster_udf
        && nopt < (derived_table + cte + table + view + engine_udf + generate_series_udf + system_table + merge_udf + cluster_udf + 1))
    {
        TableFunction * tf = tof->mutable_tfunc();
        ClusterFunc * cdf = tf->mutable_cluster();
        const uint32_t remote_table = 10 * static_cast<uint32_t>(has_table);
        const uint32_t remote_view = 5 * static_cast<uint32_t>(has_view);
        const uint32_t recurse = 10 * static_cast<uint32_t>(can_recurse);
        const uint32_t pspace = remote_table + remote_view + recurse;
        std::uniform_int_distribution<uint32_t> ndist(1, pspace);
        const uint32_t nopt2 = ndist(rg.generator);

        cdf->set_cname(static_cast<ClusterFunc_CName>((rg.nextRandomUInt32() % static_cast<uint32_t>(ClusterFunc::CName_MAX)) + 1));
        cdf->set_ccluster(rg.pickRandomly(fc.clusters));
        if (remote_table && nopt2 < (remote_table + 1))
        {
            ExprSchemaTable * est = cdf->mutable_tof()->mutable_est();
            const SQLTable & t = rg.pickRandomly(filterCollection<SQLTable>(has_table_lambda));

            est->mutable_database()->set_database("d" + (t.db ? std::to_string(t.db->dname) : "efault"));
            est->mutable_table()->set_table("t" + std::to_string(t.tname));
            if (rg.nextBool())
            {
                /// Optional sharding key
                flatTableColumnPath(to_remote_entries, t, [](const SQLColumn &) { return true; });
                cdf->set_sharding_key(rg.pickRandomly(this->remote_entries).getBottomName());
                this->remote_entries.clear();
            }
            addTableRelation(rg, false, rel_name, t);
        }
        else if (remote_view && nopt2 < (remote_table + remote_view + 1))
        {
            ExprSchemaTable * est = cdf->mutable_tof()->mutable_est();
            const SQLView & v = rg.pickRandomly(filterCollection<SQLView>(has_view_lambda));

            est->mutable_database()->set_database("d" + (v.db ? std::to_string(v.db->dname) : "efault"));
            est->mutable_table()->set_table("v" + std::to_string(v.tname));
            if (rg.nextBool())
            {
                cdf->set_sharding_key("c" + std::to_string(rg.randomInt<uint32_t>(0, 5)));
            }
            addViewRelation(rel_name, v);
        }
        else if (recurse && nopt2 < (remote_table + remote_view + recurse + 1))
        {
            /// Here don't care about the returned result
            this->depth++;
            auto u = joinedTableOrFunction(rg, rel_name, allowed_clauses, true, cdf->mutable_tof());
            UNUSED(u);
            this->depth--;
        }
        else
        {
            chassert(0);
        }
    }
    else if (
        merge_index_udf
        && nopt
            < (derived_table + cte + table + view + engine_udf + generate_series_udf + system_table + merge_udf + cluster_udf
               + merge_index_udf + 1))
    {
        SQLRelation rel(rel_name);
        TableFunction * tf = tof->mutable_tfunc();
        MergeTreeIndexFunc * mtudf = tf->mutable_mtindex();
        const SQLTable & t = rg.pickRandomlyFromVector(filterCollection<SQLTable>(has_table_lambda));

        mtudf->set_mdatabase("d" + (t.db ? std::to_string(t.db->dname) : "efault"));
        mtudf->set_mtable("t" + std::to_string(t.tname));
        if (rg.nextBool())
        {
            mtudf->set_with_marks(rg.nextBool());
        }
        rel.cols.emplace_back(SQLRelationCol(rel_name, {"part_name"}));
        rel.cols.emplace_back(SQLRelationCol(rel_name, {"mark_number"}));
        rel.cols.emplace_back(SQLRelationCol(rel_name, {"rows_in_granule"}));
        this->levels[this->current_level].rels.emplace_back(rel);
    }
    else if (
        loop_udf
        && nopt
            < (derived_table + cte + table + view + engine_udf + generate_series_udf + system_table + merge_udf + cluster_udf
               + merge_index_udf + loop_udf + 1))
    {
        /// Here don't care about the returned result
        this->depth++;
        auto u = joinedTableOrFunction(rg, rel_name, allowed_clauses, true, tof->mutable_tfunc()->mutable_loop());
        UNUSED(u);
        this->depth--;
    }
    else
    {
        chassert(0);
    }
    return set_final;
}

void StatementGenerator::generateFromElement(RandomGenerator & rg, const uint32_t allowed_clauses, TableOrSubquery * tos)
{
    JoinedTableOrFunction * jtof = tos->mutable_joined_table();
    const String name
        = fmt::format("t{}d{}", std::to_string(this->levels[this->current_level].rels.size()), std::to_string(this->current_level));

    jtof->mutable_table_alias()->set_table(name);
    jtof->set_final(joinedTableOrFunction(rg, name, allowed_clauses, false, jtof->mutable_tof()));
}

void StatementGenerator::addJoinClause(RandomGenerator & rg, BinaryExpr * bexpr)
{
    const SQLRelation * rel1 = &rg.pickRandomly(this->levels[this->current_level].rels);
    const SQLRelation * rel2 = &this->levels[this->current_level].rels.back();

    if (rel1->name == rel2->name)
    {
        rel1 = &this->levels[this->current_level].rels[this->levels[this->current_level].rels.size() - 2];
    }
    if (rg.nextSmallNumber() < 4)
    {
        /// Swap relations
        const SQLRelation * rel3 = rel1;
        rel1 = rel2;
        rel2 = rel3;
    }
    bexpr->set_op(
        rg.nextSmallNumber() < 9
            ? BinaryOperator::BINOP_EQ
            : static_cast<BinaryOperator>((rg.nextRandomUInt32() % static_cast<uint32_t>(BinaryOperator::BINOP_LEEQGR)) + 1));
    const SQLRelationCol & col1 = rg.pickRandomly(rel1->cols);
    const SQLRelationCol & col2 = rg.pickRandomly(rel2->cols);
    Expr * expr1 = bexpr->mutable_lhs();
    Expr * expr2 = bexpr->mutable_rhs();
    ExprSchemaTableColumn * estc1 = expr1->mutable_comp_expr()->mutable_expr_stc();
    ExprSchemaTableColumn * estc2 = expr2->mutable_comp_expr()->mutable_expr_stc();
    ExprColumn * ecol1 = estc1->mutable_col();
    ExprColumn * ecol2 = estc2->mutable_col();

    if (!rel1->name.empty())
    {
        estc1->mutable_table()->set_table(rel1->name);
    }
    if (!rel2->name.empty())
    {
        estc2->mutable_table()->set_table(rel2->name);
    }
    col1.AddRef(ecol1->mutable_path());
    col2.AddRef(ecol2->mutable_path());
    addFieldAccess(rg, expr1, 6);
    addFieldAccess(rg, expr2, 6);
    addColNestedAccess(rg, ecol1, 6);
    addColNestedAccess(rg, ecol2, 6);
}

void StatementGenerator::generateJoinConstraint(RandomGenerator & rg, const bool allow_using, JoinConstraint * jc)
{
    if (rg.nextSmallNumber() < 9)
    {
        bool generated = false;

        if (allow_using && rg.nextSmallNumber() < 3)
        {
            /// Using clause
            const SQLRelation & rel1 = rg.pickRandomly(this->levels[this->current_level].rels);
            const SQLRelation & rel2 = this->levels[this->current_level].rels.back();
            std::vector<DB::Strings> cols1;
            std::vector<DB::Strings> cols2;
            std::vector<DB::Strings> intersect;

            cols1.reserve(rel1.cols.size());
            for (const auto & entry : rel1.cols)
            {
                cols1.push_back(entry.path);
            }
            cols2.reserve(rel2.cols.size());
            for (const auto & entry : rel2.cols)
            {
                cols2.push_back(entry.path);
            }
            std::set_intersection(cols1.begin(), cols1.end(), cols2.begin(), cols2.end(), std::back_inserter(intersect));

            if (!intersect.empty())
            {
                ExprColumnList * ecl = jc->mutable_using_expr()->mutable_col_list();
                const uint32_t nclauses
                    = std::min<uint32_t>(UINT32_C(3), (rg.nextRandomUInt32() % static_cast<uint32_t>(intersect.size())) + 1);

                std::shuffle(intersect.begin(), intersect.end(), rg.generator);
                for (uint32_t i = 0; i < nclauses; i++)
                {
                    ColumnPath * cp = i == 0 ? ecl->mutable_col()->mutable_path() : ecl->add_extra_cols()->mutable_path();
                    const DB::Strings & npath = intersect[i];

                    for (size_t j = 0; j < npath.size(); j++)
                    {
                        Column * col = j == 0 ? cp->mutable_col() : cp->add_sub_cols();

                        col->set_column(npath[j]);
                    }
                }
                generated = true;
            }
        }
        if (!generated)
        {
            /// Joining clause
            const uint32_t nclauses = std::min(this->fc.max_width - this->width, rg.nextSmallNumber() % 3) + UINT32_C(1);
            BinaryExpr * bexpr = jc->mutable_on_expr()->mutable_comp_expr()->mutable_binary_expr();

            for (uint32_t i = 0; i < nclauses; i++)
            {
                if (i == nclauses - 1)
                {
                    addJoinClause(rg, bexpr);
                }
                else
                {
                    addJoinClause(rg, bexpr->mutable_lhs()->mutable_comp_expr()->mutable_binary_expr());
                    bexpr->set_op(rg.nextSmallNumber() < 9 ? BinaryOperator::BINOP_AND : BinaryOperator::BINOP_OR);
                    bexpr = bexpr->mutable_rhs()->mutable_comp_expr()->mutable_binary_expr();
                }
            }
        }
    }
    else
    {
        /// Random clause
        const bool prev_allow_aggregates = this->levels[this->current_level].allow_aggregates;
        const bool prev_allow_window_funcs = this->levels[this->current_level].allow_window_funcs;

        this->levels[this->current_level].allow_aggregates = this->levels[this->current_level].allow_window_funcs = false;
        generateExpression(rg, jc->mutable_on_expr());
        this->levels[this->current_level].allow_aggregates = prev_allow_aggregates;
        this->levels[this->current_level].allow_window_funcs = prev_allow_window_funcs;
    }
}

void StatementGenerator::addWhereSide(RandomGenerator & rg, const std::vector<GroupCol> & available_cols, Expr * expr)
{
    if (rg.nextSmallNumber() < 3)
    {
        refColumn(rg, rg.pickRandomly(available_cols), expr);
    }
    else
    {
        generateLiteralValue(rg, expr);
    }
}

void StatementGenerator::addWhereFilter(RandomGenerator & rg, const std::vector<GroupCol> & available_cols, Expr * expr)
{
    const GroupCol & gcol = rg.pickRandomly(available_cols);
    const uint32_t noption = rg.nextLargeNumber();

    if (noption < 761)
    {
        /// Binary expr
        BinaryExpr * bexpr = expr->mutable_comp_expr()->mutable_binary_expr();
        Expr * lexpr = bexpr->mutable_lhs();
        Expr * rexpr = bexpr->mutable_rhs();

        if (rg.nextSmallNumber() < 9)
        {
            refColumn(rg, gcol, lexpr);
            addWhereSide(rg, available_cols, rexpr);
        }
        else
        {
            addWhereSide(rg, available_cols, lexpr);
            refColumn(rg, gcol, rexpr);
        }
        bexpr->set_op(
            rg.nextSmallNumber() < 7
                ? BinaryOperator::BINOP_EQ
                : static_cast<BinaryOperator>((rg.nextRandomUInt32() % static_cast<uint32_t>(BinaryOperator::BINOP_LEGR)) + 1));
    }
    else if (noption < 901)
    {
        /// Between expr
        const uint32_t noption2 = rg.nextMediumNumber();
        ExprBetween * bexpr = expr->mutable_comp_expr()->mutable_expr_between();
        Expr * expr1 = bexpr->mutable_expr1();
        Expr * expr2 = bexpr->mutable_expr2();
        Expr * expr3 = bexpr->mutable_expr3();

        bexpr->set_not_(rg.nextBool());
        if (noption2 < 34)
        {
            refColumn(rg, gcol, expr1);
            addWhereSide(rg, available_cols, expr2);
            addWhereSide(rg, available_cols, expr3);
        }
        else if (noption2 < 68)
        {
            addWhereSide(rg, available_cols, expr1);
            refColumn(rg, gcol, expr2);
            addWhereSide(rg, available_cols, expr3);
        }
        else
        {
            addWhereSide(rg, available_cols, expr1);
            addWhereSide(rg, available_cols, expr2);
            refColumn(rg, gcol, expr3);
        }
    }
    else if (noption < 971)
    {
        /// Is null expr
        ExprNullTests * enull = expr->mutable_comp_expr()->mutable_expr_null_tests();

        enull->set_not_(rg.nextBool());
        refColumn(rg, gcol, enull->mutable_expr());
    }
    else if (noption < 981)
    {
        /// Like expr
        ExprLike * elike = expr->mutable_comp_expr()->mutable_expr_like();
        Expr * expr2 = elike->mutable_expr2();

        elike->set_not_(rg.nextBool());
        elike->set_keyword(
            static_cast<ExprLike_PossibleKeywords>((rg.nextRandomUInt32() % static_cast<uint32_t>(ExprLike::PossibleKeywords_MAX)) + 1));
        refColumn(rg, gcol, elike->mutable_expr1());
        if (rg.nextSmallNumber() < 5)
        {
            expr2->mutable_lit_val()->set_no_quote_str(rg.nextString("'", true, rg.nextRandomUInt32() % 1009));
        }
        else
        {
            addWhereSide(rg, available_cols, expr2);
        }
    }
    else if (noption < 991)
    {
        /// In expr
        const uint32_t nclauses = rg.nextSmallNumber();
        ExprIn * ein = expr->mutable_comp_expr()->mutable_expr_in();
        ExprList * elist = ein->mutable_exprs();

        ein->set_not_(rg.nextBool());
        ein->set_global(rg.nextBool());
        refColumn(rg, gcol, ein->mutable_expr()->mutable_expr());
        for (uint32_t i = 0; i < nclauses; i++)
        {
            addWhereSide(rg, available_cols, elist->mutable_expr());
        }
    }
    else
    {
        /// Any predicate
        generatePredicate(rg, expr);
    }
}

void StatementGenerator::generateWherePredicate(RandomGenerator & rg, Expr * expr)
{
    std::vector<GroupCol> available_cols;
    const uint32_t noption = rg.nextSmallNumber();

    if (this->levels[this->current_level].gcols.empty() && !this->levels[this->current_level].global_aggregate)
    {
        for (const auto & entry : this->levels[this->current_level].rels)
        {
            for (const auto & col : entry.cols)
            {
                available_cols.emplace_back(GroupCol(col, nullptr));
            }
        }
    }
    else if (!this->levels[this->current_level].gcols.empty())
    {
        for (const auto & entry : this->levels[this->current_level].gcols)
        {
            available_cols.push_back(entry);
        }
    }

    this->depth++;
    if (!available_cols.empty() && noption < 8)
    {
        const uint32_t nclauses = std::max(std::min(this->fc.max_width - this->width, (rg.nextSmallNumber() % 4) + 1), UINT32_C(1));

        for (uint32_t i = 0; i < nclauses; i++)
        {
            this->width++;
            if (i == nclauses - 1)
            {
                addWhereFilter(rg, available_cols, expr);
            }
            else
            {
                BinaryExpr * bexpr = expr->mutable_comp_expr()->mutable_binary_expr();

                addWhereFilter(rg, available_cols, bexpr->mutable_lhs());
                bexpr->set_op(rg.nextSmallNumber() < 8 ? BinaryOperator::BINOP_AND : BinaryOperator::BINOP_OR);
                expr = bexpr->mutable_rhs();
            }
        }
        this->width -= nclauses;
    }
    else if (noption < 10)
    {
        /// Predicate
        generatePredicate(rg, expr);
    }
    else
    {
        /// Random clause
        generateExpression(rg, expr);
    }
    this->depth--;
}

void StatementGenerator::generateFromStatement(RandomGenerator & rg, const uint32_t allowed_clauses, FromStatement * ft)
{
    JoinClause * jc = ft->mutable_tos()->mutable_join_clause();
    const uint32_t njoined = std::min(this->fc.max_width - this->width, (rg.nextMediumNumber() % UINT32_C(4)) + 1);

    this->depth++;
    this->width++;
    generateFromElement(rg, allowed_clauses, jc->mutable_tos());
    for (uint32_t i = 1; i < njoined; i++)
    {
        JoinClauseCore * jcc = jc->add_clauses();

        this->depth++;
        this->width++;
        if (this->width < this->fc.max_width && rg.nextSmallNumber() < 3)
        {
            generateArrayJoin(rg, jcc->mutable_arr());
        }
        else
        {
            JoinCore * core = jcc->mutable_core();
            JoinType jt = static_cast<JoinType>((rg.nextRandomUInt32() % static_cast<uint32_t>(JoinType_MAX)) + 1);

            if (!this->allow_not_deterministic && jt == JoinType::J_PASTE)
            {
                jt = JoinType::J_INNER;
            }
            core->set_global(rg.nextSmallNumber() < 3);
            core->set_join_op(jt);
            if (rg.nextSmallNumber() < 4)
            {
                switch (jt)
                {
                    case JoinType::J_LEFT:
                    case JoinType::J_INNER:
                        core->set_join_const(static_cast<JoinConst>((rg.nextRandomUInt32() % static_cast<uint32_t>(JoinConst_MAX)) + 1));
                        break;
                    case JoinType::J_RIGHT:
                        core->set_join_const(
                            static_cast<JoinConst>((rg.nextRandomUInt32() % static_cast<uint32_t>(JoinConst::J_ANTI)) + 1));
                        break;
                    case JoinType::J_FULL:
                        core->set_join_const(JoinConst::J_ALL);
                        break;
                    default:
                        break;
                }
            }
            generateFromElement(rg, allowed_clauses, core->mutable_tos());
            generateJoinConstraint(rg, njoined == 2, core->mutable_join_constraint());
        }
    }
    this->width -= njoined;
    this->depth -= njoined;
}

void StatementGenerator::generateGroupByExpr(
    RandomGenerator & rg,
    const bool enforce_having,
    const uint32_t offset,
    const uint32_t ncols,
    const std::vector<SQLRelationCol> & available_cols,
    std::vector<GroupCol> & gcols,
    Expr * expr)
{
    const uint32_t next_option = rg.nextSmallNumber();

    if (!available_cols.empty() && (enforce_having || next_option < 9))
    {
        const SQLRelationCol & rel_col = available_cols[offset];
        ExprSchemaTableColumn * estc = expr->mutable_comp_expr()->mutable_expr_stc();
        ExprColumn * ecol = estc->mutable_col();

        if (!rel_col.rel_name.empty())
        {
            estc->mutable_table()->set_table(rel_col.rel_name);
        }
        rel_col.AddRef(ecol);
        addFieldAccess(rg, expr, 6);
        addColNestedAccess(rg, ecol, 6);
        gcols.emplace_back(GroupCol(rel_col, expr));
    }
    else if (ncols && next_option < 10)
    {
        LiteralValue * lv = expr->mutable_lit_val();

        lv->mutable_int_lit()->set_uint_lit((rg.nextRandomUInt64() % ncols) + 1);
    }
    else
    {
        generateExpression(rg, expr);
    }
}

bool StatementGenerator::generateGroupBy(
    RandomGenerator & rg, const uint32_t ncols, const bool enforce_having, const bool allow_settings, GroupByStatement * gbs)
{
    std::vector<SQLRelationCol> available_cols;

    if (!this->levels[this->current_level].rels.empty())
    {
        for (const auto & entry : this->levels[this->current_level].rels)
        {
            available_cols.insert(available_cols.end(), entry.cols.begin(), entry.cols.end());
        }
        std::shuffle(available_cols.begin(), available_cols.end(), rg.generator);
    }
    if (enforce_having && available_cols.empty())
    {
        return false;
    }
    this->depth++;
    if (enforce_having || !allow_settings || rg.nextSmallNumber() < (available_cols.empty() ? 3 : 9))
    {
        std::vector<GroupCol> gcols;
        const uint32_t next_opt = rg.nextMediumNumber();
        GroupByList * gbl = gbs->mutable_glist();
        const uint32_t nccols = std::min<uint32_t>(
            UINT32_C(5), (rg.nextRandomUInt32() % (available_cols.empty() ? 5 : static_cast<uint32_t>(available_cols.size()))) + 1);
        const uint32_t nclauses = std::min<uint32_t>(this->fc.max_width - this->width, nccols);
        const bool no_grouping_sets = next_opt < 91 || !allow_settings;
        const bool has_gsm = !enforce_having && next_opt < 51 && allow_settings && rg.nextSmallNumber() < 4;
        const bool has_totals
            = !enforce_having && this->peer_query != PeerQuery::AllPeers && no_grouping_sets && allow_settings && rg.nextSmallNumber() < 4;

        if (no_grouping_sets)
        {
            /// Group list
            ExprList * elist = (!allow_settings || next_opt < 51) ? gbl->mutable_exprs()
                                                                  : ((next_opt < 71) ? gbl->mutable_rollup() : gbl->mutable_cube());

            for (uint32_t i = 0; i < nclauses; i++)
            {
                this->width++;
                generateGroupByExpr(
                    rg, enforce_having, i, ncols, available_cols, gcols, i == 0 ? elist->mutable_expr() : elist->add_extra_exprs());
            }
        }
        else
        {
            /// Grouping sets
            bool has_global = false;
            GroupingSets * gsets = gbl->mutable_sets();

            for (uint32_t i = 0; i < nclauses; i++)
            {
                const uint32_t nelems = std::min<uint32_t>(
                    this->fc.max_width - this->width,
                    rg.nextRandomUInt32() % (available_cols.empty() ? 3 : static_cast<uint32_t>(available_cols.size())));
                OptionalExprList * oel = i == 0 ? gsets->mutable_exprs() : gsets->add_other_exprs();

                has_global |= nelems == 0;
                this->width++;
                for (uint32_t j = 0; j < nelems; j++)
                {
                    this->width++;
                    generateGroupByExpr(rg, enforce_having, j, ncols, available_cols, gcols, oel->add_exprs());
                }
                this->width -= nelems;
                std::shuffle(available_cols.begin(), available_cols.end(), rg.generator);
            }
            this->levels[this->current_level].global_aggregate |= gcols.empty() && has_global;
        }
        this->width -= nclauses;
        this->levels[this->current_level].gcols = std::move(gcols);

        if (has_gsm)
        {
            gbl->set_gsm(static_cast<GroupByList_GroupingSetsModifier>(
                (rg.nextRandomUInt32() % static_cast<uint32_t>(GroupByList::GroupingSetsModifier_MAX)) + 1));
        }
        gbl->set_with_totals(has_totals);

        if (!has_gsm && !has_totals && allow_settings && (enforce_having || rg.nextSmallNumber() < 5))
        {
            const bool prev_allow_aggregates = this->levels[this->current_level].allow_aggregates;

            this->levels[this->current_level].allow_aggregates = true;
            generateWherePredicate(rg, gbs->mutable_having_expr());
            this->levels[this->current_level].allow_aggregates = prev_allow_aggregates;
        }
    }
    else
    {
        gbs->set_gall(true);
        this->levels[this->current_level].group_by_all = true;
    }
    this->depth--;
    return true;
}

void StatementGenerator::generateOrderBy(RandomGenerator & rg, const uint32_t ncols, const bool allow_settings, OrderByStatement * ob)
{
    if (allow_settings && rg.nextSmallNumber() < 3)
    {
        ob->set_oall(true);
    }
    else
    {
        bool has_fill = false;
        std::vector<GroupCol> available_cols;
        OrderByList * olist = ob->mutable_olist();

        if (this->levels[this->current_level].group_by_all)
        {
            for (const auto & entry : this->levels[this->current_level].projections)
            {
                const String cname = "c" + std::to_string(entry);
                available_cols.emplace_back(GroupCol(SQLRelationCol("", {cname}), nullptr));
            }
        }
        else if (this->levels[this->current_level].gcols.empty() && !this->levels[this->current_level].global_aggregate)
        {
            for (const auto & entry : this->levels[this->current_level].rels)
            {
                for (const auto & col : entry.cols)
                {
                    available_cols.emplace_back(GroupCol(col, nullptr));
                }
            }
        }
        else if (!this->levels[this->current_level].gcols.empty())
        {
            for (const auto & entry : this->levels[this->current_level].gcols)
            {
                available_cols.push_back(entry);
            }
        }
        if (!available_cols.empty())
        {
            std::shuffle(available_cols.begin(), available_cols.end(), rg.generator);
        }
        const uint32_t nccols = std::min<uint32_t>(
            UINT32_C(5), rg.nextRandomUInt32() % (available_cols.empty() ? 5 : static_cast<uint32_t>(available_cols.size())) + 1);
        const uint32_t nclauses = std::min<uint32_t>(this->fc.max_width - this->width, nccols);

        for (uint32_t i = 0; i < nclauses; i++)
        {
            ExprOrderingTerm * eot = i == 0 ? olist->mutable_ord_term() : olist->add_extra_ord_terms();
            Expr * expr = eot->mutable_expr();
            const uint32_t next_option = rg.nextSmallNumber();

            this->width++;
            if (!available_cols.empty() && next_option < 9)
            {
                refColumn(rg, available_cols[i], expr);
            }
            else if (ncols && next_option < 10)
            {
                LiteralValue * lv = expr->mutable_lit_val();

                lv->mutable_int_lit()->set_uint_lit((rg.nextRandomUInt64() % ncols) + 1);
            }
            else
            {
                generateExpression(rg, expr);
            }
            if (allow_settings)
            {
                if (rg.nextSmallNumber() < 7)
                {
                    eot->set_asc_desc(rg.nextBool() ? AscDesc::ASC : AscDesc::DESC);
                }
                if (rg.nextSmallNumber() < 7)
                {
                    eot->set_nulls_order(
                        rg.nextBool() ? ExprOrderingTerm_NullsOrder::ExprOrderingTerm_NullsOrder_FIRST
                                      : ExprOrderingTerm_NullsOrder::ExprOrderingTerm_NullsOrder_LAST);
                }
                if (!this->fc.collations.empty() && rg.nextSmallNumber() < 3)
                {
                    eot->set_collation(rg.pickRandomly(this->fc.collations));
                }
                if (this->fc.test_with_fill && rg.nextSmallNumber() < 2)
                {
                    const uint32_t nopt = rg.nextSmallNumber();
                    ExprOrderingWithFill * eowf = eot->mutable_fill();

                    has_fill = true;
                    if (nopt < 4)
                    {
                        generateExpression(rg, eowf->mutable_from_expr());
                    }
                    else if (nopt < 7)
                    {
                        generateExpression(rg, eowf->mutable_staleness_expr());
                    }
                    if (rg.nextSmallNumber() < 4)
                    {
                        generateExpression(rg, eowf->mutable_to_expr());
                    }
                    if (rg.nextSmallNumber() < 4)
                    {
                        generateExpression(rg, eowf->mutable_step_expr());
                    }
                }
            }
        }
        this->width -= nclauses;
        if (has_fill && !this->levels[this->current_level].projections.empty()
            && (this->levels[this->current_level].group_by_all
                || (this->levels[this->current_level].gcols.empty() && !this->levels[this->current_level].global_aggregate))
            && rg.nextSmallNumber() < 4)
        {
            std::vector<uint32_t> nids;
            const uint32_t nprojs = std::min<uint32_t>(
                UINT32_C(3), (rg.nextRandomUInt32() % static_cast<uint32_t>(this->levels[this->current_level].projections.size())) + 1);
            const uint32_t iclauses = std::min<uint32_t>(this->fc.max_width - this->width, nprojs);

            nids.insert(
                nids.end(), this->levels[this->current_level].projections.begin(), this->levels[this->current_level].projections.end());
            std::shuffle(nids.begin(), nids.end(), rg.generator);
            for (uint32_t i = 0; i < iclauses; i++)
            {
                InterpolateExpr * ie = olist->add_interpolate();

                ie->mutable_col()->set_column("c" + std::to_string(nids[i]));
                generateExpression(rg, ie->mutable_expr());
                this->width++;
            }
            this->width -= iclauses;
        }
    }
}

void StatementGenerator::generateLimitExpr(RandomGenerator & rg, Expr * expr)
{
    if (this->depth >= this->fc.max_depth || rg.nextSmallNumber() < 8)
    {
        uint32_t nlimit = 0;
        const int next_option = rg.nextSmallNumber();

        if (next_option < 3)
        {
            nlimit = 0;
        }
        else if (next_option < 5)
        {
            nlimit = 1;
        }
        else if (next_option < 7)
        {
            nlimit = 2;
        }
        else if (next_option < 9)
        {
            nlimit = 10;
        }
        else
        {
            nlimit = rg.nextRandomUInt32();
        }
        expr->mutable_lit_val()->mutable_int_lit()->set_uint_lit(nlimit);
    }
    else
    {
        this->depth++;
        generateExpression(rg, expr);
        this->depth--;
    }
}

void StatementGenerator::generateLimit(RandomGenerator & rg, const bool has_order_by, const uint32_t ncols, LimitStatement * ls)
{
    generateLimitExpr(rg, ls->mutable_limit());
    if (rg.nextBool())
    {
        generateLimitExpr(rg, ls->mutable_offset());
    }
    ls->set_with_ties(has_order_by && (!this->allow_not_deterministic || rg.nextSmallNumber() < 7));
    if (ncols && !ls->with_ties() && rg.nextSmallNumber() < 4)
    {
        Expr * expr = ls->mutable_limit_by();

        if (this->depth >= this->fc.max_depth || rg.nextSmallNumber() < 8)
        {
            LiteralValue * lv = expr->mutable_lit_val();

            lv->mutable_int_lit()->set_uint_lit((rg.nextRandomUInt64() % ncols) + 1);
        }
        else
        {
            this->depth++;
            generateExpression(rg, expr);
            this->depth--;
        }
    }
}

void StatementGenerator::generateOffset(RandomGenerator & rg, const bool has_order_by, OffsetStatement * off)
{
    generateLimitExpr(rg, off->mutable_row_count());
    off->set_rows(rg.nextBool());
    if (has_order_by && (!this->allow_not_deterministic || rg.nextBool()))
    {
        FetchStatement * fst = off->mutable_fetch();

        generateLimitExpr(rg, fst->mutable_row_count());
        fst->set_rows(rg.nextBool());
        fst->set_first(rg.nextBool());
        fst->set_only(!this->allow_not_deterministic || rg.nextBool());
    }
}

void StatementGenerator::addCTEs(RandomGenerator & rg, const uint32_t allowed_clauses, CTEs * qctes)
{
    const uint32_t nclauses = std::min<uint32_t>(this->fc.max_width - this->width, (rg.nextRandomUInt32() % 3) + 1);

    this->depth++;
    for (uint32_t i = 0; i < nclauses; i++)
    {
        CTEquery * cte = i == 0 ? qctes->mutable_cte() : qctes->add_other_ctes();
        const String name = fmt::format("cte{}d{}", std::to_string(i), std::to_string(this->current_level));
        SQLRelation rel(name);
        const uint32_t ncols = std::min(this->fc.max_width - this->width, (rg.nextMediumNumber() % UINT32_C(5)) + 1);

        cte->mutable_table()->set_table(name);
        generateDerivedTable(rg, rel, allowed_clauses, ncols, cte->mutable_query());
        this->ctes[this->current_level][name] = std::move(rel);
        this->width++;
    }
    this->width -= nclauses;
    this->depth--;
}

void StatementGenerator::generateSelect(
    RandomGenerator & rg, const bool top, bool force_global_agg, const uint32_t ncols, uint32_t allowed_clauses, Select * sel)
{
    if ((allowed_clauses & allow_cte) && this->depth < this->fc.max_depth && this->width < this->fc.max_width && rg.nextMediumNumber() < 13)
    {
        this->addCTEs(rg, allowed_clauses, sel->mutable_ctes());
    }
    if ((allowed_clauses & allow_set) && !force_global_agg && this->depth<this->fc.max_depth && this->fc.max_width> this->width + 1
        && rg.nextSmallNumber() < 3)
    {
        SetQuery * setq = sel->mutable_set_query();
        ExplainQuery * eq1 = setq->mutable_sel1();
        ExplainQuery * eq2 = setq->mutable_sel2();

        setq->set_set_op(static_cast<SetQuery_SetOp>((rg.nextRandomUInt32() % static_cast<uint32_t>(SetQuery::SetOp_MAX)) + 1));
        setq->set_s_or_d(rg.nextBool() ? AllOrDistinct::ALL : AllOrDistinct::DISTINCT);

        this->depth++;
        this->current_level++;
        if (ncols == 1 && rg.nextMediumNumber() < 6)
        {
            prepareNextExplain(rg, eq1);
        }
        else
        {
            this->levels[this->current_level] = QueryLevel(this->current_level);
            generateSelect(rg, false, false, ncols, allowed_clauses, eq1->mutable_inner_query()->mutable_select()->mutable_sel());
        }
        this->width++;
        if (ncols == 1 && rg.nextMediumNumber() < 6)
        {
            prepareNextExplain(rg, eq2);
        }
        else
        {
            this->levels[this->current_level] = QueryLevel(this->current_level);
            generateSelect(rg, false, false, ncols, allowed_clauses, eq2->mutable_inner_query()->mutable_select()->mutable_sel());
        }
        this->current_level--;
        this->depth--;
        this->width--;
    }
    else
    {
        bool force_group_by = false;
        bool force_order_by = false;
        SelectStatementCore * ssc = sel->mutable_select_core();

        if ((allowed_clauses & allow_distinct) && rg.nextSmallNumber() < 3)
        {
            ssc->set_s_or_d(rg.nextBool() ? AllOrDistinct::ALL : AllOrDistinct::DISTINCT);
        }
        if ((allowed_clauses & allow_from) && this->depth < this->fc.max_depth && this->width < this->fc.max_width
            && rg.nextSmallNumber() < 10)
        {
            generateFromStatement(rg, allowed_clauses, ssc->mutable_from());
        }
        const bool prev_allow_aggregates = this->levels[this->current_level].allow_aggregates;
        const bool prev_allow_window_funcs = this->levels[this->current_level].allow_window_funcs;

        this->levels[this->current_level].allow_aggregates = this->levels[this->current_level].allow_window_funcs = false;
        if ((allowed_clauses & allow_prewhere) && this->depth < this->fc.max_depth && ssc->has_from() && rg.nextSmallNumber() < 2)
        {
            generateWherePredicate(rg, ssc->mutable_pre_where()->mutable_expr()->mutable_expr());
        }
        if ((allowed_clauses & allow_where) && this->depth < this->fc.max_depth && rg.nextSmallNumber() < 5)
        {
            generateWherePredicate(rg, ssc->mutable_where()->mutable_expr()->mutable_expr());
        }

        if (this->inside_projection)
        {
            const uint32_t nopt = rg.nextSmallNumber();

            force_global_agg |= nopt < 4;
            force_group_by |= nopt > 3 && nopt < 7;
            if (force_global_agg || force_group_by)
            {
                allowed_clauses &= ~(allow_orderby);
            }
            else
            {
                allowed_clauses &= ~(allow_groupby | allow_global_aggregate);
                force_order_by = true;
            }
        }

        if ((allowed_clauses & allow_groupby) && !force_global_agg && this->depth < this->fc.max_depth && this->width < this->fc.max_width
            && (force_group_by || rg.nextSmallNumber() < 4))
        {
            generateGroupBy(rg, ncols, false, (allowed_clauses & allow_groupby_settings), ssc->mutable_groupby());
        }
        else
        {
            this->levels[this->current_level].global_aggregate
                = (allowed_clauses & allow_global_aggregate) && (force_global_agg || rg.nextSmallNumber() < 4);
        }
        this->levels[this->current_level].allow_aggregates = prev_allow_aggregates;
        this->levels[this->current_level].allow_window_funcs = prev_allow_window_funcs;

        this->depth++;
        for (uint32_t i = 0; i < ncols; i++)
        {
            ExprColAlias * eca = ssc->add_result_columns()->mutable_eca();

            this->width++;
            generateExpression(rg, eca->mutable_expr());
            if (!top)
            {
                const uint32_t cname = this->levels[this->current_level].aliases_counter++;
                const String cname_str = "c" + std::to_string(cname);

                SQLRelation rel("");
                rel.cols.emplace_back(SQLRelationCol("", {cname_str}));
                this->levels[this->current_level].rels.emplace_back(rel);
                eca->mutable_col_alias()->set_column(cname_str);
                this->levels[this->current_level].projections.emplace_back(cname);
            }
        }
        this->depth--;
        this->width -= ncols;

        if ((allowed_clauses & allow_orderby) && this->depth < this->fc.max_depth && this->width < this->fc.max_width
            && (!this->allow_not_deterministic || force_order_by || rg.nextSmallNumber() < 4))
        {
            this->depth++;
            generateOrderBy(rg, ncols, (allowed_clauses & allow_orderby_settings), ssc->mutable_orderby());
            this->depth--;
        }
        if ((allowed_clauses & allow_limit) && (this->allow_not_deterministic || ssc->has_orderby()) && rg.nextSmallNumber() < 4)
        {
            if (rg.nextBool())
            {
                generateLimit(rg, ssc->has_orderby(), ncols, ssc->mutable_limit());
            }
            else
            {
                generateOffset(rg, ssc->has_orderby(), ssc->mutable_offset());
            }
        }
    }
    /// This doesn't work: SELECT 1 FROM ((SELECT 1) UNION (SELECT 1) SETTINGS page_cache_inject_eviction = 1) x;
    if (this->allow_not_deterministic && !this->inside_projection && (top || sel->has_select_core()) && rg.nextSmallNumber() < 3)
    {
        generateSettingValues(rg, serverSettings, sel->mutable_setting_values());
    }
    this->levels.erase(this->current_level);
    this->ctes.erase(this->current_level);
}

void StatementGenerator::generateTopSelect(
    RandomGenerator & rg, const bool force_global_agg, const uint32_t allowed_clauses, TopSelect * ts)
{
    const uint32_t ncols = std::max(std::min(this->fc.max_width - this->width, (rg.nextMediumNumber() % UINT32_C(5)) + 1), UINT32_C(1));

    this->levels[this->current_level] = QueryLevel(this->current_level);
    generateSelect(rg, true, force_global_agg, ncols, allowed_clauses, ts->mutable_sel());
    this->levels.clear();
    if (rg.nextSmallNumber() < 3)
    {
        SelectIntoFile * sif = ts->mutable_intofile();
        const std::filesystem::path & qfile = fc.db_file_path / "file.data";

        ts->set_format(static_cast<OutFormat>((rg.nextRandomUInt32() % static_cast<uint32_t>(OutFormat_MAX)) + 1));
        sif->set_path(qfile.generic_string());
        if (rg.nextSmallNumber() < 4)
        {
            sif->set_step(SelectIntoFile_SelectIntoFileStep::SelectIntoFile_SelectIntoFileStep_TRUNCATE);
        }
        if (rg.nextSmallNumber() < 4)
        {
            sif->set_compression(static_cast<FileCompression>((rg.nextRandomUInt32() % static_cast<uint32_t>(FileCompression_MAX)) + 1));
        }
        if (rg.nextSmallNumber() < 4)
        {
            sif->set_level((rg.nextRandomUInt32() % 22) + 1);
        }
    }
}

}
