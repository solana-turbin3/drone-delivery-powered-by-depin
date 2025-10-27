#include "SolanaUtils.h"
#include "IoTxChain-lib.h"

extern IoTxChain solana;
extern String PROGRAM_ID;
extern String PUBLIC_KEY_BASE58;
extern uint8_t PRIVATE_KEY[64];

bool sendAnchorInstruction(
    const String& functionName,
    const String& pdaAddress,
    const std::vector<uint8_t>& payload,
    bool isInit
) {
    Serial.print("Sending: ");
    Serial.println(functionName);
    
    Pubkey authority = Pubkey::fromBase58(PUBLIC_KEY_BASE58);
    Keypair signer = Keypair::fromPrivateKey(PRIVATE_KEY);
    
    std::vector<uint8_t> programId = base58ToPubkey(PROGRAM_ID);
    
    std::vector<uint8_t> discriminator = solana.calculateDiscriminator(functionName.c_str());
    
    std::vector<uint8_t> data = discriminator;
    data.insert(data.end(), payload.begin(), payload.end());
    
    Pubkey pda = Pubkey::fromBase58(pdaAddress);
    
    std::vector<AccountMeta> accounts;
    
    if (isInit) {
        accounts = {
            AccountMeta::writable(authority, true),
            AccountMeta::writable(pda, false),
            AccountMeta{Pubkey::fromBase58("11111111111111111111111111111111"), false, false}
        };
    } else {
        accounts = {
            AccountMeta::writable(authority, true),
            AccountMeta::writable(pda, false)
        };
    }
    
    Instruction ix(Pubkey{programId}, accounts, data);
    
    Transaction tx;
    tx.fee_payer = authority;
    tx.recent_blockhash = solana.getLatestBlockhash();
    
    if (tx.recent_blockhash.isEmpty()) {
        Serial.println("Failed to get blockhash!");
        return false;
    }
    
    tx.add(ix);
    tx.sign({signer});
    
    String txBase64 = tx.serializeBase64();
    String txSig;
    
    if (solana.sendRawTransaction(txBase64, txSig)) {
        Serial.print("Transaction sent: ");
        Serial.println(txSig);
        
        if (solana.confirmTransaction(txSig, 5000)) {
            Serial.println("Transaction confirmed!");
            return true;
        } else {
            Serial.println("Transaction pending...");
            return false;
        }
    }
    
    Serial.println("Failed to send transaction");
    return false;
}