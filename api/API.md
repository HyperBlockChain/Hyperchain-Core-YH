# Hyper Block Chain API Ver 0.71
Prgramming interface specification of Hyper Block Chain.

## Contents

  * [Terms and Abbreviation](#terms-and-abbreviation)
  * [Block API](#block-api)
  * [Chain API](#chain-api)
  * [Consensus API](#consensus-api)
  * [Utility API](#utility-api)
  * [Node API](#node-api)


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

### GetHyperBlockHeader

Function:`obtain the block header of specified Hyper Block.`

Parameter:

* `HyperBlockID`
* `BlockHash`

Return:

```js
HyperBlockHeader / NULL 
```

### GetLocalBlockHeader  

Function:`obtain the block header of specified Local Block.`

Parameter:
* `LocalBlockID`
* `BlockHash`

Return:
```js
LocalBlockHeader / NULL
```

### GetHyperBlockPayload	

Function: `obtain the block payload of specified Hyper Block.`

Parameter:
* `HyperBlockID` 
* `BlockHash`

Return:
```js
{HyperBlockPayload, PayloadSize} / NULL
```

### GetLocalBlockPayload	

Function: `obtain the block payload of specified Local Block`

Parameter:
* `LocalBlockID` 
* `BlockHash`

Return:
```js
{LocalBlockPayload, PayloadSize} / NULL
```

### QueryHyperBlocks	

Function: `Return all Hyper Blocks in a given segment of the Chain Space and matched the keywords in Local Block Payloads.`

Parameter:
* `Keyword: string` 
* `StartBlockID：{}`
* `StartBlockHash` 
* `EndBlockID：{}` 
* `EndBlockHash` 
* `StartTime: numeric`
* `EndTime: numeric` 

Return:
```js
HyperBlockDataList={
    ListIndex,
    HyperBlockData
} []
/ NULL
```

### QueryLocalBlocks	
Function: `Return all Local Blocks in a given segment of the Chain Space and matched the keywords in Local Block Payloads.`

Parameter:
* `Keyword: string` 
* `StartBlockID：{}`
* `StartBlockHash` 
* `EndBlockID：{}` 
* `EndBlockHash` 
* `StartTime: numeric`
* `EndTime: numeric` 

Return:
```js
LoalBlockDataList={
    ListIndex,
    LocalBlockData
}[]
/ NULL
```
### GetHyperBlockFromMultipleSource	

Function: `Obtain a Hyper Block data from multiple nodes supporting Hyper Block Chain protocol.`

Parameter:
* `HyperBlockID` 
* `NodeNum` 

Return:
```js
HyperBlockNodeMap = { //Array of map between node and Block
    HyperBlockData,
    NodeDescription,
} []
/ NULL
```
### GetLocalBlockFromMultipleSource	
Function: `Obtain a Local Block data from multiple nodes supporting Hyper Block Chain protocol.`

Parameter:
* `LocalBlockID` 
* `NodeNum`

Return:
```js
LocalBlockNodeMap = { //Array of map between node and Block
    LocalBlockData,
    NodeDescription,
} []
/ NULL
```
### MakeLocalBlockPayload	
Function:	`Build user data into formatted Local Block playload.`

Parameter:
* `UserData`

Return:
```js
LocalBlockPayload  / NULL
```
### GetParentLocalBlock	
Function: `Return the parent Local Block of specified Local Block.`

Parameter:
* `LocalBlockID` 
* `BlockHash` 

Return:
```js
LocalBlockData / NULL
```
### GetChildLocalBlock	
Function: `Return the child Local Block of specified Local Block.`

Parameter:
* `LocalBlockID` 
* `BlockHash` 

Return:
```js
LocalBlockData / NULL
```
### GetBasedHyperBlockOfLocalBlock	
Function: `Return the based Hyper Block of specified Local Block.`

Parameter:
* `LocalBlockID` 
* `BlockHash` 

Return:
```js
HyperBlockData / NULL
```
### GetBasedHyperBlockOfLocalChain	

Function: `Return the based Hyper Block of a Local Chain.`

Parameter:
* `LocalChainID`
* `BlockHash` 

Return:
```js
HyperBlockData / NULL
```


## Chain API
### GetChainOfHyperBlocks	

Function: `Return all Hyper Blocks between two specified Hyper Blocks.`

Parameter:
* `StartHyperBlockID`
* `HyperBlockHash`
* `EndHyperBlockID`
* `HyperBlockHash`

Return:
```js
HyperBlockDataList={
    ListIndex, 
    HyperBlockData
}[]
/ NULL
```
### GetLocalChainsOfHyperBlock	

Function: `Return all Local Chains of specific Hyper Block`

Parameter:
* `HyperBlockID`

Return:
```js
LocalChainDataList = {
    ListIndex,
    LocalChainID, 
    LocalBlockDataList ={
        ListIndex, 
        LocalBlockData
     }[]
}[]
/ NULL
```
### GetLocalBlocksOnLocalChain	
Function: `Return all Local Blocks in specified Local Chain`

Parameter:
* `LocalChainID`

Return:
```js
LocalBlockDataList ={
    ListIndex, 
    LocalBlockData
}[] 
/ NULL
```
### GetLatestHyperBlock	
Function: `Return the highest Hyper Block in the Chain Space.`

Parameter:
* `Void`
    
Return:
```js
HyperBlockData / NULL
```    
### GetHyperBlockHeight	
Function: `Return the Hyper Block height of the Chain Space.` 

Parameter:
* `Void`
    
Return:
```js
HyperBlockHeight : numeric / -1
```    
### GetHighestHyperBlockAtLocal	
Function: `Return the highest Hyper Block cached at local.`

Parameter:
* `Void`

Return:
```js
HyperBlockData / NULL
```
### GetCachedChainSize	

Function: `Return the storage size of chain data cached at local.`

Parameter:
* `Void`

Return:
```js
Size: numeric / -1
```    
### GetTotalChainSpaceSize	

Function: `Return the total chain size of data of the Chain Space`

Parameter:
* `Void`
    
Return:
```js
Size: numeric / -1
```    

## Consensus API
### MakeRegistration	

Function: `Build user specified data and customized consensus script into an On Chain Registration data block.` 

Parameter:
* `UserData` 
* `CustomizedConsensusScript`

Return:
```js
Registration / NULL
```
### SubmitRegistration
Function: `Submit an On Chain Registration to Hyper chain network.`

Parameter:
* `Registration` 
* `TimeOut:numeric`

Return:
```js
RegistrationTX / NULL
```
### QueryRegistrationTXState	
Function: `Query On Chain Registration progress after submission.`

Parameter:
* `RegistrationTXID` 
* `Registration.selfHash`

Return:
```js
RegistrationState:{} / NULL
```
### GetLocalRegistrationTXQueue	
Function: `Query On Chain Registration waiting list at node local.`

Parameter:
* `Void`

Return:
```js
RegistrationTXQueue / NULL
```
### CreateCustomizedConsensusScript	
Function: `Create customized consensus script and cached it at local.`

Parameter:
* `ScriptType：{}`
* `Script：string` 

Return:
```js
CustomizedConsensusScript / NULL
```
### VerifyCustomizedConsensusScript	
Function: `Verify if a customized consensus script could produce expected result or not.`

Parameter:
* `CustomizedConsensusScript` 

Return:
```js
Pass / Fail
```
### SetBuddyDifficulty	

Function: `specify an expected Proof Of Work difficulty for building Local Block buddy locally.`

Parameter:
* `Difficulty: numeric` 
* `LocalBlockData`

Return:
```js
Success / Fail
```
### GetOngoingConsensusData	

Function: `Return a snapshot of the status of ongoing consensus running at local node.`

Parameter:
* `Void` 
    
Return:
```
OngoingConsensusData / NULL
```    
### GetOngoingConsensusDataElem	

Function: `Return a data item of the status of ongoing consensus running at local.`

Parameter:
* `OngoingConsensusData`
* `ConsensusDataElemKey`

Return:
```js
ConsensusDataElem / NULL
```
### GetBaseHyperBlockOfOngoingConsensus	
Function: `Return the base Hyper Block of ongoing consensus at local.`

Parameter:
* `Void`
    
Return:
```js
HyperBlockData 
```    
### GetLocalChainListOfOngoingConsensus	

Function: `Return the Local Chain list of ongoing consensus at local.`

Parameter:
* `OngoingConsensusData`

Return:
```js    
LocalChainDataList / NULL
```    
### GetLocalChainOfOngoingConsensus	
Function: `Return a Local Chain data of ongoing consensus at local.`

Parameter:
* `LocalChainID`

Return:
```js
LocalChainData / NULL
```

## Utility API

### isDataValidated	

Function: `Validate a data block with specified data type and user defined rule.`

Parameter:
* `ValidationType` 
* `Data：Void`
* `UserDefinedRuleDescription: string`
```js
ValidationType = enum { 
    HyperBlock,
    LocalBlock,
    RegistrationTX,
    CustomizedConsesusScript,
    MerkleTree,
    UserDefined 
}
```
Return:
```js
Result = {
    Code = {Pass/Fail}
    Description: string
} 
```

### MakeFormatedDataBlock	
Function:	`Build user data into data block with specified data format.`

Parameter:
* `PayloadFormat`
* `UserData` 
```js
PayloadFormat = enum { 
    LocalBlockFormat,
    RegistrationTXFormat,
    LedgerTXFormat,
    CrossChainTXFormat,
    ContractFormat,
    UserDefinedFormat
}
```
Return:
```js
FormartedDataBlock / NULL
```

## Node API

### GetNodeState	

Function: `Return Node running state.`

Parameter:
* `Void`

Return:
```js
NodeState / NULL
```
### GetNodeDescription	

Function: `Return Node Description`

Parameter:
* `Void` 

Return:
```js
NodeDescription / NULL
```
### GetNodeCapabilityDescription	

Function: `Return Node Capability Description`

Parameter:
* `Void` 

Return:
```
NodeCapabilityDescription / NULL
```
### GetConnectedNodeDescription	

Function: `Return Description of Node which have been connected with local node.`

Parameter:
* `NodeID`

Return:
```js
NodeDescription / NULL
```
### GetConnectedNodeCapabilityDescription	

Function: `Return Capability Description of Node which have been connected with local node.`

Parameter:
* `NodeID`

Return:
```js
NodeCapabilityDescription / NULL
```
### GetConnectingNodeList	

Function: `Return the list of Node which is connecting with local node.`

Parameter:
* `Void`

Return:
```js
NodeList / NULL
```
### GetConnectedNodeList	

Function: `Return the list of Node which have been connected with local node.`

Parameter:
* `Void`
    
Return:
```js
NodeList / NULL
```
### GetSendingRate	

Function: `Return current byte sending rate, Byte per Second, of local node.`

Parameter:
* `Void`

Return:
```js
SendingRate: numeric / -1
```
### GetSentBytes	

Function: `Return accumulated bytes have been sent since node started.`

Parameter:
* `Void`

Return:
```js
SentBytes: numeric / -1
```
### GetReceivingRate	

Function: Return current byte receiving rate, Byte per Second, of local node.

Parameter:
* `Void`

Return:
```js
ReceivingRate: numeric / -1
```
### GetReceivedBytes	

Function: Return accumulated bytes have been received since node started.

Parameter:
* `Void`

Return:
```js
ReceivedBytes: numeric / -1
```
### GetSentRegistrationTXsInTotal	

Function: `Return accumulated On Chain Registration have been sent from local node since node started.`

Parameter:
* `Void`

Return:
```js
SentTotal: numeric / -1
```
### GetReceivedRegistrationTXsInTotal	
Function: `Return accumulated On Chain Registration have been received from other node since local node started.`

Parameter:
* `Void`

Return:
```js
ReceivedTotal: numeric / -1
```
### GetConnectingNodeListByConnectState	
Function: `Return the list of Node which is connecting with local node by availability level.`

Parameter:
* `ConnectState`

```js
ConnectState={
    Strong,
    Average,
    Weak,
    Offline
}
/NULL
```

Return:
```js
{NodeList, Count} / NULL
```

