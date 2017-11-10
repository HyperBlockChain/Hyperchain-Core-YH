# Hyper Block Chain API
Ver 0.71


## Terms and Abbreviation
|Item	|Description  |
|-----|-------------|
|Hyper Block  	|Hyper Block is invented to resolve the contradiction between parallelized capability and traceability on continuous consistency. The block is also a forward consistent data structure and record information of Local Blocks.
|Local Block	|Local Block structure is similar to common Block Chain block, technically say, a forward dependent consistency chain structure.
|Data Block	|Ordinary concept of a package of Data with specific designed data structure
|MHT	|Merkle Hash Tree
|Registration	|A block of data contains information which a user want submit and record permanently in Chain Space.
|RegistrationTX 	|A data package contains registration which a node would broadcast to Hyper Block Chain network and record permanently in Chain Space after consensus.
|Chain Space	|Chain Space contains all chains of data of Hyper Block Chain.
|Node	|A P2P node which is running Hyper Block Chain protocol.
|Plugin	|A software program which implements HyperchainCore plugin framework and can be hooked on HyperchainCore software to provide specific extended functionality. 
|Ledger	|Similar concept to distribution ledger in common Block Chain Technology
|ContractEngine	|Configurable Hyper Block Chain Component, Interpreter and runtime executor to Block Chain Smart Contract code.
|CrossChainEngine |Hyper Block Chain Component of handling cross chain trading transactions between two local chains.


## Block API
```js
GetHyperBlockHeader  Function:obtain the block header of specified Hyper Block.
                     Parameter:
                         HyperBlockID
                         BlockHash
                     Return:
                         HyperBlockHeader / NULL 
------
GetLocalBlockHeader  Function:obtain the block header of specified Local Block.
                     Parameter:
                         LocalBlockID
                         BlockHash
                     Return:
                         LocalBlockHeader / NULL
------
GetHyperBlockPayload Function: obtain the block payload of specified Hyper Block
                     Parameter:
                         HyperBlockID 
                         BlockHash 
                     Return:
                         {HyperBlockPayload, PayloadSize} / NULL
------
GetLocalBlockPayload Function: obtain the block payload of specified Local Block
                     Parameter:
                         LocalBlockID 
                         BlockHash
                     Return:
                         {LocalBlockPayload, PayloadSize} / NULL
------
QueryHyperBlocks     Function: Return all Hyper Blocks in a given segment of the Chain Space and 
                         matched the keywords in Local Block Payloads.
                     Parameter:
                         Keyword: string 
                         StartBlockID：{}
                         StartBlockHash 
                         EndBlockID：{} 
                         EndBlockHash 
                         StartTime: numeric
                         EndTime: numeric 
                     Return:
                         HyperBlockDataList={
                             ListIndex,
                             HyperBlockData
                         } []
                         / NULL
------
QueryLocalBlocks     Function: Return all Local Blocks in a given segment of the Chain Space and 
                         matched the keywords in Local Block Payloads.
                     Parameter:
                         Keyword: string 
                         StartBlockID：{}
                         StartBlockHash 
                         EndBlockID：{} 
                         EndBlockHash 
                         StartTime: numeric
                         EndTime: numeric 
                     Return:
                         LoalBlockDataList={
                             ListIndex,
                             LocalBlockData
                         }[]
                         / NULL
------
GetHyperBlockFromMultipleSource 
                     Function: Obtain a Hyper Block data from multiple nodes supporting
                         Hyper Block Chain protocol.
                     Parameter:
                         HyperBlockID 
                         NodeNum 
                     Return:
                         HyperBlockNodeMap = { //Array of map between node and Block
                             HyperBlockData,
                             NodeDescription,
                         } []
                         / NULL
------
GetLocalBlockFromMultipleSource
                     Function: Obtain a Local Block data from multiple nodes supporting
                         Hyper Block Chain protocol.
                     Parameter:
                         LocalBlockID 
                     NodeNum
                         Return:
                         LocalBlockNodeMap = { //Array of map between node and Block
                             LocalBlockData,
                             NodeDescription,
                         } []
                         / NULL
------
MakeLocalBlockPayload Function:	Build user data into formatted Local Block playload.
                     Parameter:
                         UserData： 
                     Return:
                         LocalBlockPayload  / NULL
------
GetParentLocalBlock  Function: Return the parent Local Block of specified Local Block
                     Parameter:
                         LocalBlockID 
                         BlockHash 
                     Return:
                         LocalBlockData / NULL
------
GetChildLocalBlock   Function: Return the child Local Block of specified Local Block
                     Parameter:
                         LocalBlockID 
                         BlockHash 
                     Return:
                         LocalBlockData / NULL
------
GetBasedHyperBlockOfLocalBlock
                     Function: Return the based Hyper Block of specified Local Block 
                     Parameter:
                         LocalBlockID 
                         BlockHash 
                     Return:
                         HyperBlockData / NULL
------
GetBasedHyperBlockOfLocalChain 
                     Function: Return the based Hyper Block of a Local Chain
                     Parameter:
                         LocalChainID 
                         BlockHash 
                     Return:
                         HyperBlockData / NULL
------
```
---
