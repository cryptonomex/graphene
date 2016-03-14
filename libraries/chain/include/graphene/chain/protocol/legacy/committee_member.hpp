
namespace graphene { namespace chain { namespace legacy {

/**
 * @brief Used by committee_members to update the global parameters of the blockchain.
 * @ingroup operations
 *
 * This operation allows the committee_members to update the global parameters on the blockchain. These control various
 * tunable aspects of the chain, including block and maintenance intervals, maximum data sizes, the fees charged by
 * the network, etc.
 *
 * This operation may only be used in a proposed transaction, and a proposed transaction which contains this
 * operation must have a review period specified in the current global parameters before it may be accepted.
 *
 * This operation is deprecated in favor of committee_member_change_parameter_operation and committee_member_change_fee_operation.
 */
struct committee_member_update_global_parameters_operation : public base_operation
{
   struct fee_parameters_type { uint64_t fee = GRAPHENE_BLOCKCHAIN_PRECISION; };

   asset             fee;
   chain_parameters  new_parameters;

   account_id_type fee_payer()const { return account_id_type(); }
   void            validate()const;
};

} } } // graphene::chain::legacy

FC_REFLECT( graphene::chain::legacy::committee_member_update_global_parameters_operation::fee_parameters_type, (fee) )

FC_REFLECT( graphene::chain::legacy::committee_member_update_global_parameters_operation, (fee)(new_parameters) );
