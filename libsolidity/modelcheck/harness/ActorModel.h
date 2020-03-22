/**
 * Provides interfaces to describe contract-based actors, and to maintain the
 * corresponding model in code.
 * @date 2020
 */

#pragma once

#include <libsolidity/ast/ASTVisitor.h>
#include <libsolidity/modelcheck/codegen/Details.h>
#include <libsolidity/modelcheck/utils/General.h>

#include <list>
#include <map>
#include <memory>
#include <vector>

namespace dev
{
namespace solidity
{
namespace modelcheck
{

class AddressSpace;
class FunctionSpecialization;
class MapIndexSummary;
class NewCallGraph;
class StateGenerator;
class TypeConverter;

// -------------------------------------------------------------------------- //

/**
 * A structure used to summarize each contract in the system. This includes
 * metadata and also declarations.
 */
struct Actor
{
    Actor(
        TypeConverter const& _converter,
        ContractDefinition const* _contract,
        CExprPtr _path,
        TicketSystem<uint16_t> & _cids,
        TicketSystem<uint16_t> & _fids
    );

    // The underlying contract.
    ContractDefinition const* contract;

    // Specializations of all member funtions
    std::list<FunctionSpecialization> specs;

    // A variable declaration used to maintain the actor in the harness.
    std::shared_ptr<CVarDecl> decl;

    // Assigns a unique number to each function (relative to all other actors).
    std::map<FunctionDefinition const*, size_t> fnums;

    // Associates each function parameter with the variable used to store its
    // parameters.
    std::map<VariableDeclaration const*, std::shared_ptr<CVarDecl>> fparams;

    // Maintains a path of accesses, from parent contract decl to child contract
    // decl.
    CExprPtr path;
};

// -------------------------------------------------------------------------- //

/**
 * Provides utilities to summarize, initialize, and maintain contract instances,
 * known as actors.
 */
class ActorModel
{
public:
    ActorModel(
        std::list<ContractDefinition const *> _model,
        TypeConverter const& _converter,
        MapIndexSummary const& _addrdata
    );

    // Aggregates all contract in case no model is provided.
    void record(std::vector<ContractDefinition const*> _new);

    // Integrates all contracts (either model, or aggregate) into a single
    // summary of actors. Once invoked calls to record are disallowed while
    // calls to all other interfaces are allowed.
    void setup(NewCallGraph const& _allocs);

    // Writes a declaration for each actor.
    void declare(CBlockList & _block) const;

    // Writes an initialization call to _block, for each actor. Calls are
    // performed with non-deterministic parameters.
    void initialize(
        CBlockList & _block,
        CallState const& m_statedata,
        StateGenerator const& m_stategen
    ) const;

    // Generates statements to allocate addresses for each actor.
    void assign_addresses(CBlockList & _block, AddressSpace & _addrspace) const;

    // Returns a list of contract address declarations.
    std::list<CExprPtr> const& vars() const;

    // Allow read-only access to this contract's actors
    std::list<Actor> const& inspect() const;

private:
    // Flag variable used to signal when setup is over.
    bool m_finalized = false;

    // If a model was given explicitly, it is recorded by this list. When the
    // list is empty no model has been given, and the default model is used (one
    // top-level instance of each contract).
    std::list<ContractDefinition const *> m_model;

    // The list of actors, which is populated after setup.
    std::list<Actor> m_actors;

    // A list of all contracts presented in the source.
    std::list<ContractDefinition const*> m_contracts;

    // An anonymous list of contract address member variables.
    std::list<CExprPtr> m_addrvar;

	TypeConverter const& m_converter;
    MapIndexSummary const& m_addrdata;

    // Extends setup to children. _path will accumulate the path to the current
    // parent, starting from a top level contract.
    void recursive_setup(
        NewCallGraph const& _allocs,
        CExprPtr _path,
        ContractDefinition const* _parent,
        TicketSystem<uint16_t> & _cids,
        TicketSystem<uint16_t> & _fids
    );
};

// -------------------------------------------------------------------------- //

}
}
}