#include <graphene/chain/database.hpp>
#include <graphene/chain/crosschain_trx_object.hpp>
#include <graphene/chain/contract.hpp>
#include <graphene/chain/storage.hpp>
#include <graphene/chain/contract_entry.hpp>
#include <graphene/chain/transaction_object.hpp>
#include <graphene/chain/contract_object.hpp>
#include <uvm/uvm_api.h>

namespace graphene {
	namespace chain {
		StorageDataType database::get_contract_storage(const address& contract_id, const string& name)
		{
			try {
				auto& storage_index = get_index_type<contract_storage_object_index>().indices().get<by_contract_id_storage_name>();
				auto& storage_iter = storage_index.find(boost::make_tuple(contract_id, name));
				if (storage_iter == storage_index.end())
				{
					std::string null_jsonstr("null");
					return StorageDataType(null_jsonstr);
				}
				else
				{
					const auto &storage_data = *storage_iter;
					StorageDataType storage;
					storage.storage_data = storage_data.storage_value;
					return storage;
				}
			} FC_CAPTURE_AND_RETHROW((contract_id)(name));
		}

		void database::set_contract_storage(const address& contract_id, const string& name, const StorageDataType &value)
		{
			try {
				/*auto& index = get_index_type<contract_object_index>().indices().get<by_contract_id>();
				auto itr = index.find(contract_id);
				FC_ASSERT(itr != index.end());*/

				auto& storage_index = get_index_type<contract_storage_object_index>().indices().get<by_contract_id_storage_name>();
				auto& storage_iter = storage_index.find(boost::make_tuple(contract_id, name));
				if (storage_iter == storage_index.end()) {
					create<contract_storage_object>([&](contract_storage_object & obj) {
						obj.contract_address = contract_id;
						obj.storage_name = name;
						obj.storage_value = value.storage_data;
					});
				}
				else {
					modify(*storage_iter, [&](contract_storage_object& obj) {
						obj.storage_value = value.storage_data;
					});
				}
			} FC_CAPTURE_AND_RETHROW((contract_id)(name)(value));
		}

		void database::set_contract_storage_in_contract(const contract_object& contract, const string& name, const StorageDataType& value)
		{
			try {
				set_contract_storage(contract.contract_address, name, value);
			} FC_CAPTURE_AND_RETHROW((contract.contract_address)(name)(value));
		}

		void database::add_contract_storage_change(const address& contract_id, const string& name, const StorageDataType &diff)
		{
			try {
				transaction_contract_storage_diff_object obj;
				obj.contract_address = contract_id;
				obj.storage_name = name;
				obj.diff = diff.storage_data;
				auto& con_db = get_index_type<transaction_contract_storage_diff_index>().indices().get<by_contract_id>();
				auto con = con_db.find(contract_id);
				create<transaction_contract_storage_diff_object>([&](transaction_contract_storage_diff_object & o) {
					o.contract_address = obj.contract_address;
					o.diff = obj.diff;
					o.storage_name = obj.storage_name;
					o.trx_id = obj.trx_id;
				});
			} FC_CAPTURE_AND_RETHROW((contract_id)(name)(diff));
		}

        void database::store_contract(const contract_object & contract)
        {
            try {
            auto& con_db = get_index_type<contract_object_index>().indices().get<by_contract_id>();
            auto con = con_db.find(contract.contract_address);
            if (con == con_db.end())
            {
                create<contract_object>([contract](contract_object & obj) {
                    obj.create_time = contract.create_time;
                    obj.code = contract.code; 
                    obj.name = contract.name;
                    obj.owner_address = contract.owner_address;
                    obj.contract_address = contract.contract_address;
                });
            }
            else
            {
                FC_ASSERT( false,"contract exsited");
            }
            }FC_CAPTURE_AND_RETHROW((contract))
        }

        contract_object database::get_contract(const address & contract_address)
        {
            auto& index = get_index_type<contract_object_index>().indices().get<by_contract_id>();
            auto itr = index.find(contract_address);
            FC_ASSERT(itr != index.end());
            return *itr;
        }

        contract_object database::get_contract(const contract_id_type & id)
        {
            auto& index = get_index_type<contract_object_index>().indices().get<by_id>();
            auto itr = index.find(id);
            FC_ASSERT(itr != index.end());
            return *itr;
        }

		bool database::has_contract(const address& contract_address)
		{
			auto& index = get_index_type<contract_object_index>().indices().get<by_contract_id>();
			auto itr = index.find(contract_address);
			return itr != index.end();
		}

	}
}