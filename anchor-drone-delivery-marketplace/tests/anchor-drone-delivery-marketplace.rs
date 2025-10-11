#[cfg(test)]

mod tests {
  use {
    mollusk_svm::Mollusk,
    solana_sdk::{account::Account, instruction::{AccountMeta, Instruction}, pubkey::Pubkey},
  };

    const ID: Pubkey = pubkey!("9zVb1etgUuQp2PSTgYUAEC3dCrqM9rJVAQxkg6uXnsTF");
    const ADMIN: Pubkey = Pubkey::new_from_array([0x01; 32]);

    #[test]
    fn test_initialize() {
      let mollusk = Mollusk::new(&ID, "target/deploy/anchor-drone-delivery-marketplace.so");
     
      let mut context = Context::new(ID, None, None);
  }



//pubkeys
let program_id = Pubkey::new_unique();
let key1 = Pubkey::new_unique();
let key2 = Pubkey::new_unique();

//mollusk instance

// build the accounts
  //inject data to the accounts

//get the accounts meta
let instruction = Instruction::new_with_bytes(
  program_id,
  &[],
  vec![
      AccountMeta::new(key1, false),
      AccountMeta::new_readonly(key2, false),
  ],
);
//data 

//build the transaction

//get tx accounts



let accounts = vec![
  (key1, Account::default()),
  (key2, Account::default()),
];

let mollusk = Mollusk::new(&program_id, "my_program");

//process our instruction
// Execute the instruction and get the result.
let result = mollusk.process_instruction(&instruction, &accounts);