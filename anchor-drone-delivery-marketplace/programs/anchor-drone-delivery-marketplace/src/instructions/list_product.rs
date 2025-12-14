use anchor_lang::prelude::*;

    use crate::state::Product;

#[derive(Accounts)]
#[instruction(product_name: String)]
pub struct ListProduct<'info> {
    #[account(mut)]
    pub seller: Signer<'info>,
    #[account(
        init,
        payer = seller,
        seeds = [b"product", product_name.as_bytes()],
        bump,
        space = 8 + Product::INIT_SPACE
    )]
    pub product: Account<'info, Product>,
    pub system_program: Program<'info, System>,

}

impl <'info> ListProduct<'info> {
    pub fn list_product(&mut self, product_name: String, product_description: String, product_price: u32, bumps: &ListProductBumps) -> Result<()> {
        assert!(product_name.len() < 32);
        assert!(product_description.len() < 32);
        assert!(product_price > 0);

        self.product.set_inner(Product{
            name: product_name,
            description: product_description,
            price: product_price, 
            quantity: 1,
            seller: self.seller.key(),
            bump: bumps.product,
        });

        msg!("You listed a product named: {}", self.product.name);
        
        Ok(())
    }
}